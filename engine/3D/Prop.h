#pragma once
#include "Object3d.h"
#include "WorldTransform.h"
#include <memory>
#include <string>

class Prop {
public:
    
    void Initialize(const std::string& modelName);

   
    void Update();
    void Draw();

    void Finalize();

    Vector3 GetScale() const { return worldTransform_->scale_; }
    Vector3 GetRotate() const { return worldTransform_->rotate_; }
    Vector3 GetTranslate() const { return worldTransform_->translate_; }

    void SetScale(const Vector3& scale) { worldTransform_->scale_ = scale; }
    void SetRotate(const Vector3& rotate) { worldTransform_->rotate_ = rotate; }
    void SetTranslate(const Vector3& translate) { worldTransform_->translate_ = translate; }

    WorldTransform& GetWorldTransform() { return *worldTransform_; }
    void SetCamera(Camera* camera) {
        if (model_) {
            model_->SetCamera(camera);
        }
    }


    bool GetUseDirectionalLight() const { return model_ ? model_->GetUseDirectionalLight() : false; }
    void SetUseDirectionalLight(bool use) { if (model_) model_->SetUseDirectionalLight(use); }

    bool GetUsePointLight() const { return model_ ? model_->GetUsePointLight() : false; }
    void SetUsePointLight(bool use) { if (model_) model_->SetUsePointLight(use); }

    bool GetUseSpotLight() const { return model_ ? model_->GetUseSpotLight() : false; }
    void SetUseSpotLight(bool use) { if (model_) model_->SetUseSpotLight(use); }

    bool GetUseAmbientLight() const { return model_ ? model_->GetUseAmbientLight() : false; }
    void SetUseAmbientLight(bool use) { if (model_) model_->SetUseAmbientLight(use); }

    bool GetUseAreaLight() const { return model_ ? model_->GetUseAreaLight() : false; }
    void SetUseAreaLight(bool use) { if (model_) model_->SetUseAreaLight(use); }

private:
    
    std::unique_ptr<Object3d> model_;
    std::unique_ptr<WorldTransform> worldTransform_;
};