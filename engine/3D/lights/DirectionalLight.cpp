#include "DirectionalLight.h"
#include <cassert>
#include <d3dx12.h>
#include "ResourceUtils.h"

void DirectionalLight::Initialize(ID3D12Device* device) {

    constBuffer_ = ResourceUtils::CreateBufferResource(device, sizeof(DirectionalLightData));

    // 2. CPU 맵핑
    HRESULT hr = constBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&data_));
    assert(SUCCEEDED(hr));

    // 2. 매핑
    constBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&data_));

    // 3. 기본값 셋팅
    cpuData_.color = { 1.0f, 1.0f, 1.0f, 1.0f };
    cpuData_.direction = { 0.0f, -1.0f, 0.0f }; // 아래를 향하는 빛
    cpuData_.intensity = 1.0f;

    Update();
}

void DirectionalLight::Update() {
    if (data_ != nullptr) {
        memcpy(data_, &cpuData_, sizeof(DirectionalLightData));
    }
}

void DirectionalLight::Bind(ID3D12GraphicsCommandList* commandList, uint32_t rootParameterIndex) {
    // GPU에 이 빛의 데이터를 연결해줍니다.
    commandList->SetGraphicsRootConstantBufferView(rootParameterIndex, constBuffer_->GetGPUVirtualAddress());
}