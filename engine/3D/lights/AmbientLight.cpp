#include "AmbientLight.h"
#include <cassert>
#include <d3dx12.h>
#include "ResourceUtils.h"

void AmbientLight::Initialize(ID3D12Device* device) {

    constBuffer_ = ResourceUtils::CreateBufferResource(device, sizeof(AmbientLightData));

    
    HRESULT hr = constBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&data_));
    assert(SUCCEEDED(hr));

    hr = constBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&data_));
    assert(SUCCEEDED(hr));

  
    cpuData_.color = { 1.0f, 1.0f, 1.0f, 1.0f }; 

    Update();
}

void AmbientLight::Update() {

    if (data_ != nullptr) {
        memcpy(data_, &cpuData_, sizeof(AmbientLightData));
    }

}

void AmbientLight::Bind(ID3D12GraphicsCommandList* commandList, uint32_t rootParameterIndex) {
    commandList->SetGraphicsRootConstantBufferView(rootParameterIndex, constBuffer_->GetGPUVirtualAddress());
}