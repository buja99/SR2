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
    assert(object3dCommon != nullptr && "Object3dCommon is not initialized.");
}



void ModelManager::LoadModel(const std::string& directorypath, const std::string& filePath)
{

    std::string fullPath = directorypath + "/" + filePath;

    if (models_.contains(fullPath)) {
        return; 
    }

    
    std::unique_ptr<IModel> model;

    
    if (filePath.find(".gltf") != std::string::npos || filePath.find(".fbx") != std::string::npos) {
        model = std::make_unique<AnimatedModel>();  
    } else {
        model = std::make_unique<StaticModel>();   
    }

    
    model->Initialize(object3dCommon, directorypath, filePath);

    
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
