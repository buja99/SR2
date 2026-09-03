#pragma once
#include "IModel.h"
#include "VertexData.h" 
#include "Skeleton.h"
#include "SkinCluster.h"
#include "Animator.h"
#include "ModelTypes.h"
#include "Material.h"
#include "TextureManager.h"

class AnimatedModel : public IModel {
public:
    void Initialize(Object3dCommon* object3dCommon, const std::string& directorypath, const std::string& filename) override;

   
    void Update(float deltaTime) override;

    void Draw(const Matrix4x4& worldMatrix, const Matrix4x4& viewProj, TransformationMatrix* transformData) override;
    void Cleanup() override;

private:
    std::vector<VertexDataAnimated> vertices_;

   
    Skeleton skeleton_;
    SkinCluster skinCluster_;
    Animator animator_;
    Object3dCommon* object3dCommon_ = nullptr;
    ModelData modelData;

    ComPtr<ID3D12Resource> vertexResource_;
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};

    ComPtr<ID3D12Resource> indexResource_;
    D3D12_INDEX_BUFFER_VIEW indexBufferView_{};

    ComPtr<ID3D12Resource> materialResource_;
    Material* materialData_ = nullptr;

    // 내부 도우미 함수들
    void InitializeVertexBuffer();
    void InitializeIndexBuffer();
    void InitializeMaterial();
    void DrawRecursive(const Node& node, const Matrix4x4& parentMatrix, const Matrix4x4& viewProj, TransformationMatrix* transformData);
    
    ModelData LoadAnimatedModelFile(const std::string& directoryPath, const std::string& filename);

    ModelData ParseAnimatedModelData(const aiScene* scene);

};

