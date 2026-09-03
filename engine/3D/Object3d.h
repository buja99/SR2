#pragma once
#include "MyMath.h"
#include <wrl.h>
#include "WinApp.h"
#include <Windows.h>
#include "DirectXTex.h"
#include "Transform.h"
#include "Material.h"
#include "VertexData.h"
#include "TextureManager.h"
#include "IModel.h"
#include "ModelManager.h"
#include "Camera.h"
#include "Matrix4x4.h"
#include "WorldTransform.h"

class WorldTransform;
class Object3dCommon;

using Microsoft::WRL::ComPtr;

class Object3d
{
	public:
		~Object3d();

	void Initialize(Object3dCommon* object3dCommon, WorldTransform* worldTransform);
	
	void Update();
	void Draw();
	void Cleanup();

	void InitializeMaterial();

	void SetModel(IModel* model) { this->model_ = model; }

	void SetModel(const std::string& filePath);

	void SetEnvironmentMap(const std::string& filePath);
	void SetTextureDDS2D(const std::string& filePath);

	// setter
	void SetScale(const Vector3& scale) { transform.scale = scale; }
	void SetRotate(const Vector3& rotate) { transform.rotate = rotate; }
	void SetTranslate(const Vector3& translate) { transform.translate = translate; }
	void SetCamera(Camera* camera) { this->camera = camera; }
	void SetDefaultCamera(Camera* camera) { this->defaultCamera = camera; }
	// getter
	const Vector3& GetScale() const { return transform.scale; }
	const Vector3& GetRotate() const { return transform.rotate; }
	const Vector3& GetTranslate() const { return transform.translate; }
	Camera* GetDefaultCamera() const { return defaultCamera; }
	IModel* GetModel() const { return model_; }

	// Lighting setter/getter
    void SetEnableLighting(bool enable);
    bool GetEnableLighting() const;

    void SetIsBlinnPhong(bool isBlinn);
    bool GetIsBlinnPhong() const;

    void SetUsePointLight(bool use);
    bool GetUsePointLight() const;

    void SetUseDirectionalLight(bool use);
    bool GetUseDirectionalLight() const;

	void SetUseSpotLight(bool use);
	bool GetUseSpotLight() const;

	void SetUseAmbientLight(bool use);
	bool GetUseAmbientLight() const;

	void SetUseAreaLight(bool use);
	bool GetUseAreaLight() const;

	void SetUseEnvironmentMap(bool use);
	bool GetUseEnvironmentMap() const;

private:

	std::string modelName_;

	IModel* model_ = nullptr;

	Object3dCommon* object3dCommon_ = nullptr;

	ComPtr<ID3D12Resource> transformationMatrixResource;
	TransformationMatrix* transformationMatrixData = nullptr;

	ComPtr<ID3D12Resource> cameraResource_;
	CameraForGPU* cameraData_ = nullptr;

	//void CreateVertexBuffer();
	//void InitializeMaterial();
	void InitializeTransformationMatrix();

	Transform transform;
	Transform cameraTransform;

	Camera* camera = nullptr;
	Camera* defaultCamera = nullptr;

	WorldTransform* worldTransform_ = nullptr;

	ComPtr<ID3D12Resource> materialResource_;
	Material* materialData_ = nullptr;

	D3D12_GPU_DESCRIPTOR_HANDLE envMapSrvHandle_{};

	std::optional<std::string> overrideTexturePath_;
};

