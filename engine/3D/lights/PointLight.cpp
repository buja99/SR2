#include "PointLight.h"
#include <cassert>
#include <d3dx12.h>
#include "ResourceUtils.h"

void PointLight::Initialize(ID3D12Device* device) {
    
    constBuffer_ = ResourceUtils::CreateBufferResource(device, sizeof(PointLightData));

    HRESULT hr = constBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&data_));
    assert(SUCCEEDED(hr));

    // 2. CPU에서 접근 가능하도록 맵핑
    hr = constBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&data_));
    assert(SUCCEEDED(hr));

    // 3. 기본값 세팅 (Object3d.cpp에 있던 초기값과 동일)
    cpuData_.color = { 1.0f, 1.0f, 1.0f, 1.0f };
    cpuData_.position = { 0.0f, 5.0f, 0.0f };
    cpuData_.intensity = 1.0f;
    cpuData_.radius = 10.0f;
    cpuData_.decay = 1.0f;

    Update();
}

void PointLight::Update() {
    if (data_ != nullptr) {
        memcpy(data_, &cpuData_, sizeof(PointLightData));
    }
}

void PointLight::Bind(ID3D12GraphicsCommandList* commandList, uint32_t rootParameterIndex) {
    commandList->SetGraphicsRootConstantBufferView(rootParameterIndex, constBuffer_->GetGPUVirtualAddress());
}