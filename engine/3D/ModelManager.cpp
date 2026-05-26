#include "ModelManager.h"
#include "DirectXCommon.h"
#include "IModel.h"
#include "StaticModel.h"
#include "ModelCommon.h"
#include "AnimatedModel.h"

ModelManager* ModelManager::instance = nullptr;

ModelManager* ModelManager::GetInstance()
{
    if (!instance) {
        instance = new ModelManager();
    }
    return instance;
}



void ModelManager::Initialize(DirectXCommon* dxCommon)
{
	

    modelCommon = std::make_unique<ModelCommon>();
    modelCommon->Initialize(dxCommon);

    object3dCommon = Object3dCommon::GetInstance();
    assert(object3dCommon != nullptr && "Object3dCommon이 초기화되지 않았습니다!");
}



void ModelManager::LoadModel(const std::string& directorypath, const std::string& filePath)
{

    std::string fullPath = directorypath + "/" + filePath;

    if (models_.contains(fullPath)) {
        return; // 이미 로드된 모델이면 무시
    }

    // 🌟 1. IModel 포인터 준비
    std::unique_ptr<IModel> model;

    // 🌟 2. [다형성의 핵심] 파일 확장자를 검사해서 알맞은 자식 객체를 생성!
    // 보통 애니메이션이 포함된 포맷은 .gltf 또는 .fbx 이므로 이를 기준으로 분기합니다.
    if (filePath.find(".gltf") != std::string::npos || filePath.find(".fbx") != std::string::npos) {
        model = std::make_unique<AnimatedModel>(); // 뼈대가 있는 움직이는 모델 생성!
    } else {
        model = std::make_unique<StaticModel>();   // (.obj 등) 멈춰있는 배경/사물 모델 생성!
    }

    // 🌟 3. 초기화 (이전에 ModelCommon 인자는 IModel에서 지웠으므로 object3dCommon만 넘김)
    model->Initialize(object3dCommon, directorypath, filePath);

    // 4. Map에 저장
    models_.insert(std::make_pair(filePath, std::move(model)));
}

IModel* ModelManager::FindModel(const std::string& filePath)
{
    if (models_.contains(filePath)) {
        return models_.at(filePath).get();
    }
    return nullptr;
}

void ModelManager::Finalize()
{
    for (auto& [_, model] : models_) {
        model->Cleanup(); 
    }
    models_.clear();
    
    if (instance) {
        delete instance;
        instance = nullptr;
    }

}
