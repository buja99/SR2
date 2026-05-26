#include "AnimatedModel.h"
#include "Object3dCommon.h"
#include "ResourceUtils.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

void AnimatedModel::Initialize(Object3dCommon* object3dCommon, const std::string& directorypath, const std::string& filename) {
    this->object3dCommon_ = object3dCommon;

    // 1. 모델 데이터 로드 (뼈대 정보 및 정점 가중치 포함)
    // modelData = LoadAnimatedModelFile(directorypath, filename); // ⚠️ 이 부분은 곧 새로 만들어야 합니다!

    // 2. 버퍼 생성
    // InitializeVertexBuffer();
    // InitializeIndexBuffer();
    // InitializeMaterial();

    // 3. 텍스처 로드
    // TextureManager::GetInstance()->LoadTexture(modelData.material.textureFilePath);

    // 4. 애니메이션 핵심 객체 초기화 (추후 Assimp Scene 객체를 받아와서 처리)
    // skeleton_.Initialize(scene);
    // skinCluster_.Initialize(object3dCommon_->GetDxCommon(), skeleton_.bones.size());

    // 5. 첫 번째 애니메이션 자동 재생 (테스트용)
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

    // 🌟 1. [다형성 핵심] 애니메이션 전용 파이프라인 세팅!
    commandList->SetPipelineState(object3dCommon_->GetGraphicsPipelineStateAnimated().Get());

    // 🌟 2. [다형성 핵심] 루트 파라미터 11번에 SkinCluster의 GPU 주소(뼈대 행렬들) 바인딩!
    commandList->SetGraphicsRootShaderResourceView(11, skinCluster_.GetGPUVirtualAddress());

    // 3. 기존과 동일한 렌더링 재귀 호출 (이 안에서 버텍스 버퍼 등을 세팅하고 그립니다)
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
    if (modelData.vertices.empty()) return;

    auto device = object3dCommon_->GetDxCommon()->GetDevice();
    const size_t vertexCount = modelData.vertices.size();

    // 🌟 [핵심] 애니메이션 전용 정점 구조체의 크기를 곱해줍니다.
    const UINT bufferSize = static_cast<UINT>(sizeof(VertexDataAnimated) * vertexCount);

    // 버퍼 생성 (Object3dCommon을 통해 만들거나 직접 CreateBufferResource 구현 활용)
    vertexResource_ = ResourceUtils::CreateBufferResource(device.Get(), bufferSize);

    vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
    vertexBufferView_.SizeInBytes = bufferSize;
    vertexBufferView_.StrideInBytes = sizeof(VertexDataAnimated);

    // 데이터 복사
    VertexDataAnimated* mapped = nullptr;
    HRESULT hr = vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&mapped));
    assert(SUCCEEDED(hr));
    memcpy(mapped, modelData.vertices.data(), bufferSize);
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

    // 기본값 세팅
    materialData_->color = { 1.0f, 1.0f, 1.0f, 1.0f };
    materialData_->enableLighting = true; // 애니메이션 모델은 보통 빛을 받습니다.
    materialData_->uvTransform = MyMath::MakeIdentity4x4();
    materialData_->shininess = 50.0f;
}

void AnimatedModel::DrawRecursive(const Node& node, const Matrix4x4& parentMatrix, const Matrix4x4& viewProj, TransformationMatrix* transformData) {
    // 1. 현재 노드의 월드 행렬 계산 (부모 행렬과 곱함)
    Matrix4x4 currentWorldMatrix = MyMath::Multiply(node.localMatrix, parentMatrix);

    auto commandList = object3dCommon_->GetCommandList();

    // 2. 이 노드에 연결된 메쉬가 있다면 그리기
    for (uint32_t meshIndex : node.meshIndices) {
        const SubMesh& subMesh = modelData.subMeshes[meshIndex];

        // 정점 & 인덱스 버퍼 세팅
        commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);
        commandList->IASetIndexBuffer(&indexBufferView_);

        // 텍스처 세팅
        auto textureDescriptorHandle = TextureManager::GetInstance()->GetSrvHandleGPU(modelData.material.textureFilePath);
        commandList->SetGraphicsRootDescriptorTable(2, textureDescriptorHandle);

        // 그리기 (DrawIndexedInstanced)
        commandList->DrawIndexedInstanced(subMesh.indexCount, 1, subMesh.indexStart, 0, 0);
    }

    // 3. 자식 노드들이 있다면 재귀적으로 파고들며 반복
    for (const Node& child : node.children) {
        DrawRecursive(child, currentWorldMatrix, viewProj, transformData);
    }
}

ModelData AnimatedModel::LoadAnimatedModelFile(const std::string& directoryPath, const std::string& filename) {
    ModelData parsedData;
    Assimp::Importer importer;
    std::string fullPath = directoryPath + "/" + filename;

    // LimitBoneWeights 플래그: 정점당 가중치가 4개를 넘지 않도록 자동으로 필터링해줍니다!
    const aiScene* scene = importer.ReadFile(fullPath.c_str(),
        aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace | aiProcess_LimitBoneWeights);
    assert(scene && scene->HasMeshes());

    // 1. 🌟 뼈대(Skeleton) 구조를 가장 먼저 구축합니다.
    skeleton_.Initialize(scene);

    // 2. 메쉬(Mesh) 순회
    for (uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex) {
        aiMesh* mesh = scene->mMeshes[meshIndex];
        uint32_t baseVertex = static_cast<uint32_t>(parsedData.verticesAnimated.size());

        // --- [A] 정점 기본 데이터 읽기 ---
        for (uint32_t v = 0; v < mesh->mNumVertices; ++v) {
            VertexDataAnimated vertex{};
            vertex.position = { mesh->mVertices[v].x, mesh->mVertices[v].y, mesh->mVertices[v].z, 1.0f };
            if (mesh->HasTextureCoords(0)) {
                vertex.texCoord = { mesh->mTextureCoords[0][v].x, mesh->mTextureCoords[0][v].y };
            }
            if (mesh->HasNormals()) {
                vertex.normal = { mesh->mNormals[v].x, mesh->mNormals[v].y, mesh->mNormals[v].z };
            }

            // 가중치와 본 인덱스 0으로 초기화
            for (int i = 0; i < 4; ++i) {
                vertex.weight[i] = 0.0f;
                vertex.boneIndices[i] = 0;
            }
            parsedData.verticesAnimated.push_back(vertex);
        }

        // --- [B] 인덱스 데이터 읽기 ---
        for (uint32_t f = 0; f < mesh->mNumFaces; ++f) {
            aiFace face = mesh->mFaces[f];
            for (uint32_t i = 0; i < face.mNumIndices; ++i) {
                parsedData.indices.push_back(baseVertex + face.mIndices[i]);
            }
        }

        // 🌟 --- [C] 스키닝 데이터 (뼈 가중치) 읽기 --- 🌟
        for (uint32_t b = 0; b < mesh->mNumBones; ++b) {
            aiBone* bone = mesh->mBones[b];
            std::string boneName = bone->mName.C_Str();

            // 생성해둔 Skeleton에서 현재 뼈의 인덱스 찾기
            auto it = skeleton_.boneIndexMap.find(boneName);
            if (it == skeleton_.boneIndexMap.end()) continue;
            int boneIndex = it->second;

            // Offset 행렬(Inverse Bind Pose) 저장
            aiMatrix4x4 aiOffset = bone->mOffsetMatrix;
            aiOffset.Transpose(); // DirectX 규격(Row-major)으로 전치
            skeleton_.bones[boneIndex].offsetMatrix = MyMath::ConvertMatrix(aiOffset);

            // 이 뼈가 영향을 주는 정점들을 찾아가서 가중치와 인덱스 기록!
            for (uint32_t w = 0; w < bone->mNumWeights; ++w) {
                uint32_t vertexID = baseVertex + bone->mWeights[w].mVertexId;
                float weight = bone->mWeights[w].mWeight;

                VertexDataAnimated& vertex = parsedData.verticesAnimated[vertexID];

                // 정점이 가진 4개의 슬롯 중 빈 곳(0.0f)을 찾아 기록합니다.
                for (int i = 0; i < 4; ++i) {
                    if (vertex.weight[i] == 0.0f) {
                        vertex.weight[i] = weight;
                        vertex.boneIndices[i] = boneIndex;
                        break;
                    }
                }
            }
        }

        // (참고: 서브메쉬(SubMesh)를 나눈다면 여기서 추가 처리)
        SubMesh subMesh{};
        subMesh.indexStart = static_cast<uint32_t>(parsedData.indices.size() - (mesh->mNumFaces * 3));
        subMesh.indexCount = mesh->mNumFaces * 3;
        parsedData.subMeshes.push_back(subMesh);
    }

    // --- 애니메이션 데이터 로드 (필요하다면 기존 방식과 유사하게 구현) ---
    // parsedData.animations = LoadAnimations(scene); 

    // 임시 머티리얼 텍스처 파일명 세팅 (기존 로직 맞춰서 수정 가능)
    parsedData.material.textureFilePath = "resources/uvChecker.png";

    return parsedData;
}
