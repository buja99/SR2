#include "Prop.h"
#include "Object3dCommon.h"

void Prop::Initialize(const std::string& modelName) {
    // 1. 위치(Transform) 부품 생성 및 초기화
    worldTransform_ = std::make_unique<WorldTransform>();
    worldTransform_->Initialize();

    // 2. 3D 모델(Object3d) 부품 생성 및 조립
    model_ = std::make_unique<Object3d>();
    // 여기서 모델과 트랜스폼을 연결해 줍니다!
    model_->Initialize(Object3dCommon::GetInstance(), worldTransform_.get());
    model_->SetModel(modelName);
}

void Prop::Update() {
    // 모델의 Update만 호출하면 알아서 애니메이션과 행렬이 계산됩니다.
    model_->Update();
}

void Prop::Draw() {
    // 그리기
    model_->Draw();
}

void Prop::Finalize() {
    // 모델이 존재한다면 내부의 버퍼와 메모리를 안전하게 해제합니다.
    if (model_) {
        model_->Cleanup();
    }
}