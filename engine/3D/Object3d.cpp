#define NOMINMAX
#include <Windows.h>
#include "Object3d.h"
#include "Object3dCommon.h"
#include <fstream>
#ifdef _DEBUG
#include "imgui.h"
#endif // _DEBUG
#include "TextureUploader.h"
#include "StringUtility.h"
#include "LightManager.h"
#include "ResourceUtils.h"

using namespace StringUtility;

Object3d::~Object3d() {
	//OutputDebugStringA("Object3d Destructor Called\n");
	Cleanup();
}

void Object3d::Initialize(Object3dCommon* object3dCommon, WorldTransform* worldTransform)
{
	assert(object3dCommon != nullptr);
	assert(worldTransform != nullptr);

	this->object3dCommon_ = object3dCommon;
	worldTransform_ = worldTransform;

	transform = { {1.0f,1.0f,1.0f},{0.0f,3.14f,0.0f},{0.0f,0.0f,10.0f} };
	cameraTransform = { {1.0f,1.0f,1.0f},{0.3f,0.0f,0.0f},{0.0f,4.0f,-10.0f} };

	cameraResource_ = ResourceUtils::CreateBufferResource(object3dCommon_->GetDxCommon()->GetDevice(), sizeof(CameraForGPU));
	cameraResource_->Map(0, nullptr, reinterpret_cast<void**>(&cameraData_));

	InitializeTransformationMatrix();
	InitializeMaterial();
	//materialData_->useEnvironmentMap = 0;

	this->camera = object3dCommon->GetDefaultCamera();
}

void Object3d::Update() {
	if (!worldTransform_) {
		return;
	}

	// 🌟 [다형성 핵심 1] 옛날의 길고 복잡한 애니메이션 로직은 전부 삭제!
	// 이제 모델이 스스로 업데이트하도록 시간(deltaTime)만 던져주면 끝입니다.
	if (model_) {
		model_->Update(1.0f / 60.0f); // (AnimatedModel이면 애니가 재생되고, Static이면 무시됨)
	}

	// Object3d의 transform 값을 worldTransform에 전달
	worldTransform_->scale_ = transform.scale;
	worldTransform_->rotate_ = transform.rotate;
	worldTransform_->translate_ = transform.translate;

	// 월드 행렬 갱신
	worldTransform_->UpdateMatrix();

	// 🌟 [다형성 핵심 2] WVP 및 행렬 계산 간소화
	// 이전에는 Object3d가 GetModelData().rootNode.localMatrix를 강제로 가져와서 곱했지만,
	// 이제는 모델 내부의 DrawRecursive() 함수가 알아서 localMatrix를 곱해줍니다!
	// 따라서 Object3d는 아주 단순하게 자신의 World 행렬만 세팅해두면 됩니다.
	if (camera) {
		const Matrix4x4& viewProj = camera->GetViewProjectionMatrix();

		transformationMatrixData->WVP = MyMath::Multiply(worldTransform_->matWorld_, viewProj);
		transformationMatrixData->World = worldTransform_->matWorld_;
		transformationMatrixData->WorldInverseTranspose = MyMath::Transpose(MyMath::Inverse(worldTransform_->matWorld_));
	}

	// 카메라 위치 갱신 (반사/조명용)
	if (camera && cameraData_) {
		cameraData_->worldPosition = camera->GetEye();
	}

#ifdef _DEBUG
	// 디버그 UI 등이 필요하다면 여기에 추가
#endif // _DEBUG
}

void Object3d::Draw()
{
	if (!model_ || !object3dCommon_ || !camera || !worldTransform_) return;

	auto commandList = object3dCommon_->GetCommandList();

	// 1. 공통 셰이더 리소스 레지스터 바인딩 (Material, Transform, Camera, Light 등)
	commandList->SetGraphicsRootConstantBufferView(0, materialResource_.Get()->GetGPUVirtualAddress());
	commandList->SetGraphicsRootConstantBufferView(1, transformationMatrixResource.Get()->GetGPUVirtualAddress());
	commandList->SetGraphicsRootConstantBufferView(4, cameraResource_->GetGPUVirtualAddress());
	LightManager::GetInstance()->BindAll(commandList.Get());

	// 2. 뷰 프로젝션 행렬 획득
	const Matrix4x4& viewProj = camera->GetViewProjectionMatrix();

	// 
	model_->Draw(worldTransform_->matWorld_, viewProj, transformationMatrixData);
}

void Object3d::Cleanup()
{

	if (transformationMatrixResource) {
		transformationMatrixResource.Reset();
		transformationMatrixData = nullptr;
	}

	if (cameraResource_) {
		cameraResource_.Reset();         
		cameraData_ = nullptr;
	}
	if (materialResource_) {
		materialResource_.Reset();             
		materialData_ = nullptr;               
	}
	
	object3dCommon_ = nullptr;
	camera = nullptr;
	defaultCamera = nullptr;
	transformationMatrixData = nullptr;
	cameraData_ = nullptr;
}

void Object3d::InitializeMaterial() {

	auto device = object3dCommon_->GetDxCommon()->GetDevice();
	materialResource_ = ResourceUtils::CreateBufferResource(device, sizeof(Material));
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));

	// 조명 및 머티리얼 초기 설정
	materialData_->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	materialData_->enableLighting = false;
	materialData_->uvTransform = MyMath::MakeIdentity4x4();
	materialData_->shininess = 32.0f;
	materialData_->isBlinnPhong = 0;
	materialData_->usePointLight = 0;
	materialData_->useDirectionalLight = 1;
	materialData_->useSpotLight = 0;
	materialData_->useAmbientLight = 0;
	materialData_->useAreaLight = 0;
}




void Object3d::SetModel(const std::string& filePath)
{
	modelName_ = filePath;  
	model_ = ModelManager::GetInstance()->FindModel(filePath);
	if (!model_) {
		OutputDebugStringA(("Model not found: " + filePath + "\n").c_str());
	}
}

void Object3d::SetEnvironmentMap(const std::string& filePath) {
	// DDS 큐브맵 로드
	TextureManager::GetInstance()->LoadTextureDDS(filePath, true);

	// GPU 핸들 얻기
	D3D12_GPU_DESCRIPTOR_HANDLE handle =
		TextureManager::GetInstance()->GetSrvHandleGPU(filePath);

	// 핸들 보관
	envMapSrvHandle_ = handle;

	// Material 플래그 켜기
	if (materialData_) {
		materialData_->useEnvironmentMap = 1;
	}

	
}

void Object3d::SetTextureDDS2D(const std::string& filePath) {
	TextureManager::GetInstance()->LoadTextureDDS(filePath, false);
	overrideTexturePath_ = filePath;

	if (materialData_) {

		materialData_->useEnvironmentMap = 0;
	}
}



void Object3d::SetEnableLighting(bool enable) {
	if (materialData_) materialData_->enableLighting = enable;
}

bool Object3d::GetEnableLighting() const {
	return materialData_ ? materialData_->enableLighting != 0 : false;
}

void Object3d::SetIsBlinnPhong(bool isBlinn) {
	if (materialData_) materialData_->isBlinnPhong = isBlinn;
}

bool Object3d::GetIsBlinnPhong() const {
	return materialData_ ? materialData_->isBlinnPhong != 0 : false;
}

void Object3d::SetUsePointLight(bool use) {
	if (materialData_) materialData_->usePointLight = use;
}

bool Object3d::GetUsePointLight() const {
	return materialData_ ? materialData_->usePointLight != 0 : false;
}

void Object3d::SetUseDirectionalLight(bool use) {
	if (materialData_) materialData_->useDirectionalLight = use;
}

bool Object3d::GetUseDirectionalLight() const {
	return materialData_ ? materialData_->useDirectionalLight != 0 : false;
}

void Object3d::SetUseSpotLight(bool use) {
	if (materialData_) materialData_->useSpotLight = use;
}

bool Object3d::GetUseSpotLight() const {
	return materialData_ ? materialData_->useSpotLight != 0 : false;
}

void Object3d::SetUseAmbientLight(bool use) {
	if (materialData_) materialData_->useAmbientLight = use;
}

bool Object3d::GetUseAmbientLight() const {
	return materialData_ ? materialData_->useAmbientLight != 0 : false;
}

void Object3d::SetUseAreaLight(bool use) {
	if (materialData_) materialData_->useAreaLight = use;
}

bool Object3d::GetUseAreaLight() const {
	return materialData_ ? materialData_->useAreaLight != 0 : false;
}

void Object3d::SetUseEnvironmentMap(bool use) {
	if (materialData_) {
		materialData_->useEnvironmentMap = use ? 1 : 0;
	}
}

bool Object3d::GetUseEnvironmentMap() const {
	return materialData_ ? (materialData_->useEnvironmentMap != 0) : false;
}




void Object3d::InitializeTransformationMatrix()
{

	auto device = object3dCommon_->GetDxCommon()->GetDevice();

	transformationMatrixResource = ResourceUtils::CreateBufferResource(device.Get(), sizeof(TransformationMatrix));

	transformationMatrixResource.Get()->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData));

	transformationMatrixData->WVP = MyMath::MakeIdentity4x4();
	transformationMatrixData->World = MyMath::MakeIdentity4x4();
}


