#include "DirectionalLight.h"
#include <cassert>
#include <d3dx12.h>
#include "ResourceUtils.h"

void DirectionalLight::Initialize(ID3D12Device* device) {

    constBuffer_ = ResourceUtils::CreateBufferResource(device, sizeof(DirectionalLightData));

   
    HRESULT hr = constBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&data_));
    assert(SUCCEEDED(hr));

    
    constBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&data_));

    
    cpuData_.color = { 1.0f, 1.0f, 1.0f, 1.0f };
    cpuData_.direction = { 0.0f, -1.0f, 0.0f }; 
    cpuData_.intensity = 1.0f;

    Update();
}

void DirectionalLight::Update() {
    if (data_ != nullptr) {
        memcpy(data_, &cpuData_, sizeof(DirectionalLightData));
    }
}

void DirectionalLight::Bind(ID3D12GraphicsCommandList* commandList, uint32_t rootParameterIndex) {
    
    commandList->SetGraphicsRootConstantBufferView(rootParameterIndex, constBuffer_->GetGPUVirtualAddress());
}