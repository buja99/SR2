#include "LightManager.h"
#include <cassert>

LightManager* LightManager::GetInstance() {
    static LightManager instance;
    return &instance;
}

void LightManager::Initialize(ID3D12Device* device) {
    assert(device != nullptr);
    device_ = device;
    lights_.clear();
}

void LightManager::Update() {
    for (auto& entry : lights_) {
        entry.light->Update();
    }
}

void LightManager::BindAll(ID3D12GraphicsCommandList* commandList) {
    for (auto& entry : lights_) {
        entry.light->Bind(commandList, entry.rootParameterIndex);
    }
}

void LightManager::AddLight(std::unique_ptr<ILight> light, uint32_t rootParameterIndex) {
    if (device_ && light) {
        // 매니저에 빛이 들어오는 순간 GPU 버퍼를 생성(Initialize)합니다.
        light->Initialize(device_);
    }
    lights_.push_back({ std::move(light), rootParameterIndex });
}

void LightManager::ClearLights() {
    lights_.clear();
}