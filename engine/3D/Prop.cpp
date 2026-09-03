#include "Prop.h"
#include "Object3dCommon.h"

void Prop::Initialize(const std::string& modelName) {
    
    worldTransform_ = std::make_unique<WorldTransform>();
    worldTransform_->Initialize();

    
    model_ = std::make_unique<Object3d>();
    
    model_->Initialize(Object3dCommon::GetInstance(), worldTransform_.get());
    model_->SetModel(modelName);
}

void Prop::Update() {
   
    model_->Update();
}

void Prop::Draw() {
    
    model_->Draw();
}

void Prop::Finalize() {
    
    if (model_) {
        model_->Cleanup();
    }
}