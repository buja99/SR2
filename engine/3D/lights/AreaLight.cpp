#include "AreaLight.h"
#include <cassert>
#include <d3dx12.h>
#include "ResourceUtils.h"

void AreaLight::Initialize(ID3D12Device* device) {

    constBuffer_ = ResourceUtils::CreateBufferResource(device, sizeof(AreaLightData));

    
    HRESULT hr = constBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&data_));
    assert(SUCCEEDED(hr));

    hr = constBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&data_));
    assert(SUCCEEDED(hr));

    
    cpuData_.color = { 1.0f, 1.0f, 1.0f, 1.0f };
    cpuData_.position = { 0.0f, 5.0f, 0.0f };
    cpuData_.intensity = 1.0f;
    cpuData_.right = { 1.0f, 0.0f, 0.0f };
    cpuData_.halfWidth = 5.0f;
    cpuData_.up = { 0.0f, 0.0f, 1.0f };
    cpuData_.halfHeight = 5.0f;

    Update();
}

void AreaLight::Update() {
    if (data_ != nullptr) {
        memcpy(data_, &cpuData_, sizeof(AreaLightData));
    }
}

void AreaLight::Bind(ID3D12GraphicsCommandList* commandList, uint32_t rootParameterIndex) {
    commandList->SetGraphicsRootConstantBufferView(rootParameterIndex, constBuffer_->GetGPUVirtualAddress());
}