#pragma once
#include "ModelCommon.h"
#include <wrl.h>
#include "VertexData.h"
#include "Material.h"
#include "TextureManager.h"
#include "MyMath.h"
#include "Object3dCommon.h"
#include "IModel.h"
#include "Skeleton.h"
#include "SkinCluster.h"
#include "Animator.h"
#include "ModelTypes.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

class Object3dCommon;

class StaticModel : public IModel
{

public:
	void Initialize(Object3dCommon* object3dCommon ,const std::string& directorypath, const std::string& filename) override;

	void Update(float deltaTime) override { /* Do nothing */ }

	void Draw(const Matrix4x4& worldMatrix, const Matrix4x4& viewProj, TransformationMatrix* transformData) override;

	void Cleanup() override;

	static ModelData LoadModelFile(const std::string& directoryPath, const std::string& filename);

	static MaterialData LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename);

	ComPtr<ID3D12Resource> CreateBufferResource(ComPtr <ID3D12Device> device, size_t sizeInBytes);

	Material GetMaterialData() const { return *materialData_; }

	static Node ReadNode(aiNode* ainode);

	const ModelData& GetModelData() const { return modelData; }

	void DrawRecursive(const Node& node, const Matrix4x4& parentMatrix, const Matrix4x4& viewProj, TransformationMatrix* transformData);

	
	void InitializeIndexBuffer(const ModelData& modelData);

private:
	
	// Obj file
	std::vector<VertexDataStatic> vertices_;
	MaterialData material_;
	//
	ComPtr<ID3D12Resource> vertexResource_;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_;
	ComPtr<ID3D12Resource> materialResource_;
	Material* materialData_ = nullptr;


	ModelData modelData;

	void InitializeVertexBuffer();
	void InitializeMaterial();

	Object3dCommon* object3dCommon_ = nullptr;

	ComPtr<ID3D12Resource>      indexResource_;    
	D3D12_INDEX_BUFFER_VIEW     indexBufferView_{};

	Skeleton skeleton_;
	SkinCluster skinCluster_;
	Animator animator_;

};

