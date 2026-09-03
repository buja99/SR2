#include "AnimatedModel.h"
#include "Object3dCommon.h"
#include "ResourceUtils.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

void AnimatedModel::Initialize(Object3dCommon* object3dCommon, const std::string& directorypath, const std::string& filename) {
    this->object3dCommon_ = object3dCommon;
    std::string fullPath = directorypath + "/" + filename;

    
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(fullPath.c_str(),
        aiProcess_FlipWindingOrder | aiProcess_FlipUVs | aiProcess_Triangulate);

    assert(scene != nullptr && scene->HasMeshes() && "Error: Failed to load the animated model");

    ParseAnimatedModelData(scene);

    
    InitializeVertexBuffer(); 
    InitializeIndexBuffer();
    InitializeMaterial();

    if (!modelData.material.textureFilePath.empty()) {
        TextureManager::GetInstance()->LoadTexture(modelData.material.textureFilePath);
    }

    
    skeleton_.Initialize(scene);
    skinCluster_.Initialize(object3dCommon_->GetDxCommon(), skeleton_.bones.size());

   
    // if (!modelData.animations.empty()) {
    //     animator_.Play(modelData.animations[0], true);
    // }
}

void AnimatedModel::Update(float deltaTime) {
    animator_.Update(deltaTime, skeleton_);

    skinCluster_.Update(skeleton_);
}

void AnimatedModel::Draw(const Matrix4x4& worldMatrix, const Matrix4x4& viewProj, TransformationMatrix* transformData) {
    if (!object3dCommon_) return;
    auto commandList = object3dCommon_->GetCommandList();

    commandList->SetPipelineState(object3dCommon_->GetGraphicsPipelineStateAnimated().Get());

    commandList->SetGraphicsRootShaderResourceView(11, skinCluster_.GetGPUVirtualAddress());

    DrawRecursive(modelData.rootNode, worldMatrix, viewProj, transformData);
}

void AnimatedModel::Cleanup() {
    if (vertexResource_) vertexResource_.Reset();
    if (indexResource_) indexResource_.Reset();
    if (materialResource_) {
        materialResource_.Reset();
        materialData_ = nullptr;
    }
    animator_.Stop();
    object3dCommon_ = nullptr;
}

void AnimatedModel::InitializeVertexBuffer() {
    if (modelData.verticesAnimated.empty()) {
        return;
    }

    auto device = object3dCommon_->GetDxCommon()->GetDevice();

    const size_t vertexCount =
        modelData.verticesAnimated.size();

    const UINT bufferSize =
        static_cast<UINT>(
            sizeof(VertexDataAnimated) * vertexCount
            );

    vertexResource_ =
        ResourceUtils::CreateBufferResource(
            device.Get(),
            bufferSize
        );

    vertexBufferView_.BufferLocation =
        vertexResource_->GetGPUVirtualAddress();

    vertexBufferView_.SizeInBytes = bufferSize;
    vertexBufferView_.StrideInBytes =
        sizeof(VertexDataAnimated);

    VertexDataAnimated* mapped = nullptr;

    HRESULT hr = vertexResource_->Map(
        0,
        nullptr,
        reinterpret_cast<void**>(&mapped)
    );

    assert(SUCCEEDED(hr));

    memcpy(
        mapped,
        modelData.verticesAnimated.data(),
        bufferSize
    );

    vertexResource_->Unmap(0, nullptr);
}

void AnimatedModel::InitializeIndexBuffer() {
    if (modelData.indices.empty()) return;

    auto device = object3dCommon_->GetDxCommon()->GetDevice();
    const size_t indexCount = modelData.indices.size();
    const UINT bufferSize = static_cast<UINT>(sizeof(uint32_t) * indexCount);

    indexResource_ = ResourceUtils::CreateBufferResource(device.Get(), bufferSize);

    indexBufferView_.BufferLocation = indexResource_->GetGPUVirtualAddress();
    indexBufferView_.SizeInBytes = bufferSize;
    indexBufferView_.Format = DXGI_FORMAT_R32_UINT;

    uint32_t* mapped = nullptr;
    HRESULT hr = indexResource_->Map(0, nullptr, reinterpret_cast<void**>(&mapped));
    assert(SUCCEEDED(hr));
    memcpy(mapped, modelData.indices.data(), bufferSize);
    indexResource_->Unmap(0, nullptr);
}

void AnimatedModel::InitializeMaterial() {
    auto device = object3dCommon_->GetDxCommon()->GetDevice();

    materialResource_ = ResourceUtils::CreateBufferResource(device.Get(), sizeof(Material));
    materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));

    materialData_->color = { 1.0f, 1.0f, 1.0f, 1.0f };
    materialData_->enableLighting = true; 
    materialData_->uvTransform = MyMath::MakeIdentity4x4();
    materialData_->shininess = 50.0f;
}

void AnimatedModel::DrawRecursive(const Node& node, const Matrix4x4& parentMatrix, const Matrix4x4& viewProj, TransformationMatrix* transformData) {
    
    Matrix4x4 currentWorldMatrix = MyMath::Multiply(node.localMatrix, parentMatrix);

    auto commandList = object3dCommon_->GetCommandList();

    for (uint32_t meshIndex : node.meshIndices) {
        const SubMesh& subMesh = modelData.subMeshes[meshIndex];

        
        commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);
        commandList->IASetIndexBuffer(&indexBufferView_);

       
        auto textureDescriptorHandle = TextureManager::GetInstance()->GetSrvHandleGPU(modelData.material.textureFilePath);
        commandList->SetGraphicsRootDescriptorTable(2, textureDescriptorHandle);

       
        commandList->DrawIndexedInstanced(subMesh.indexCount, 1, subMesh.indexStart, 0, 0);
    }


    for (const Node& child : node.children) {
        DrawRecursive(child, currentWorldMatrix, viewProj, transformData);
    }
}

ModelData AnimatedModel::LoadAnimatedModelFile(const std::string& directoryPath, const std::string& filename) {
    ModelData parsedData;
    Assimp::Importer importer;
    std::string fullPath = directoryPath + "/" + filename;


    const aiScene* scene = importer.ReadFile(fullPath.c_str(),
        aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace | aiProcess_LimitBoneWeights);
    assert(scene && scene->HasMeshes());


    skeleton_.Initialize(scene);


    for (uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex) {
        aiMesh* mesh = scene->mMeshes[meshIndex];
        uint32_t baseVertex = static_cast<uint32_t>(parsedData.verticesAnimated.size());


        for (uint32_t v = 0; v < mesh->mNumVertices; ++v) {
            VertexDataAnimated vertex{};
            vertex.position = { mesh->mVertices[v].x, mesh->mVertices[v].y, mesh->mVertices[v].z, 1.0f };
            if (mesh->HasTextureCoords(0)) {
                vertex.texCoord = { mesh->mTextureCoords[0][v].x, mesh->mTextureCoords[0][v].y };
            }
            if (mesh->HasNormals()) {
                vertex.normal = { mesh->mNormals[v].x, mesh->mNormals[v].y, mesh->mNormals[v].z };
            }


            for (int i = 0; i < 4; ++i) {
                vertex.weight[i] = 0.0f;
                vertex.boneIndices[i] = 0;
            }
            parsedData.verticesAnimated.push_back(vertex);
        }


        for (uint32_t f = 0; f < mesh->mNumFaces; ++f) {
            aiFace face = mesh->mFaces[f];
            for (uint32_t i = 0; i < face.mNumIndices; ++i) {
                parsedData.indices.push_back(baseVertex + face.mIndices[i]);
            }
        }


        for (uint32_t b = 0; b < mesh->mNumBones; ++b) {
            aiBone* bone = mesh->mBones[b];
            std::string boneName = bone->mName.C_Str();


            auto it = skeleton_.boneIndexMap.find(boneName);
            if (it == skeleton_.boneIndexMap.end()) continue;
            int boneIndex = it->second;


            aiMatrix4x4 aiOffset = bone->mOffsetMatrix;
            aiOffset.Transpose(); 
            skeleton_.bones[boneIndex].offsetMatrix = MyMath::ConvertMatrix(aiOffset);


            for (uint32_t w = 0; w < bone->mNumWeights; ++w) {
                uint32_t vertexID = baseVertex + bone->mWeights[w].mVertexId;
                float weight = bone->mWeights[w].mWeight;

                VertexDataAnimated& vertex = parsedData.verticesAnimated[vertexID];

                for (int i = 0; i < 4; ++i) {
                    if (vertex.weight[i] == 0.0f) {
                        vertex.weight[i] = weight;
                        vertex.boneIndices[i] = boneIndex;
                        break;
                    }
                }
            }
        }


        SubMesh subMesh{};
        subMesh.indexStart = static_cast<uint32_t>(parsedData.indices.size() - (mesh->mNumFaces * 3));
        subMesh.indexCount = mesh->mNumFaces * 3;
        parsedData.subMeshes.push_back(subMesh);
    }


    parsedData.material.textureFilePath = "resources/uvChecker.png";

    return parsedData;
}

ModelData AnimatedModel::ParseAnimatedModelData(const aiScene* scene) {
    ModelData parsedData;


    skeleton_.Initialize(scene);


    for (uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex) {
        aiMesh* mesh = scene->mMeshes[meshIndex];
        uint32_t baseVertex = static_cast<uint32_t>(parsedData.verticesAnimated.size());


        for (uint32_t v = 0; v < mesh->mNumVertices; ++v) {
            VertexDataAnimated vertex{};
            vertex.position = { mesh->mVertices[v].x, mesh->mVertices[v].y, mesh->mVertices[v].z, 1.0f };
            if (mesh->HasTextureCoords(0)) {
                vertex.texCoord = { mesh->mTextureCoords[0][v].x, mesh->mTextureCoords[0][v].y };
            }
            if (mesh->HasNormals()) {
                vertex.normal = { mesh->mNormals[v].x, mesh->mNormals[v].y, mesh->mNormals[v].z };
            }

            for (int i = 0; i < 4; ++i) {
                vertex.weight[i] = 0.0f;
                vertex.boneIndices[i] = 0;
            }
            parsedData.verticesAnimated.push_back(vertex);
        }


        for (uint32_t f = 0; f < mesh->mNumFaces; ++f) {
            aiFace face = mesh->mFaces[f];
            for (uint32_t i = 0; i < face.mNumIndices; ++i) {
                parsedData.indices.push_back(baseVertex + face.mIndices[i]);
            }
        }


        for (uint32_t b = 0; b < mesh->mNumBones; ++b) {
            aiBone* bone = mesh->mBones[b];
            std::string boneName = bone->mName.C_Str();

           
            auto it = skeleton_.boneIndexMap.find(boneName);
            if (it == skeleton_.boneIndexMap.end()) continue;
            int boneIndex = it->second;

            
            aiMatrix4x4 aiOffset = bone->mOffsetMatrix;
            aiOffset.Transpose(); 
            skeleton_.bones[boneIndex].offsetMatrix = MyMath::ConvertMatrix(aiOffset);

           
            for (uint32_t w = 0; w < bone->mNumWeights; ++w) {
                uint32_t vertexID = baseVertex + bone->mWeights[w].mVertexId;
                float weight = bone->mWeights[w].mWeight;

                VertexDataAnimated& vertex = parsedData.verticesAnimated[vertexID];

                
                for (int i = 0; i < 4; ++i) {
                    if (vertex.weight[i] == 0.0f) {
                        vertex.weight[i] = weight;
                        vertex.boneIndices[i] = boneIndex;
                        break;
                    }
                }
            }
        }

        SubMesh subMesh{};
        subMesh.indexStart = static_cast<uint32_t>(parsedData.indices.size() - (mesh->mNumFaces * 3));
        subMesh.indexCount = mesh->mNumFaces * 3;
        parsedData.subMeshes.push_back(subMesh);
    }

   
    parsedData.material.textureFilePath = "resources/uvChecker.png";

    return parsedData;
}