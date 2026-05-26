#include "SpotLight.h"
#include <cassert>
#include <d3dx12.h>
#include <cmath> // cosf 함수용
#include "ResourceUtils.h"

void SpotLight::Initialize(ID3D12Device* device) {
    // 1. 상수 버퍼 생성 (256바이트 정렬)
    constBuffer_ = ResourceUtils::CreateBufferResource(device, sizeof(SpotLightData));

    HRESULT hr = constBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&data_));
    assert(SUCCEEDED(hr));

    // 2. CPU 맵핑
    hr = constBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&data_));
    assert(SUCCEEDED(hr));

    // 3. 기본값 세팅 (Object3d.cpp에 있던 초기값과 동일)
    cpuData_.color = { 1.0f, 1.0f, 1.0f, 1.0f };
    cpuData_.position = { 0.0f, 5.0f, 5.0f };
    cpuData_.direction = { 0.0f, -1.0f, -1.0f };
    cpuData_.intensity = 1.0f;
    cpuData_.cutoff = cosf(MyMath::ToRadian(15.0f));      // 내부 각도
    cpuData_.outerCutoff = cosf(MyMath::ToRadian(30.0f)); // 외부 각도
    cpuData_.radius = 15.0f;
    cpuData_.decay = 1.0f;

    Update();
}

void SpotLight::Update() {
    if (data_ != nullptr) {
        memcpy(data_, &cpuData_, sizeof(SpotLightData));
    }
}

void SpotLight::Bind(ID3D12GraphicsCommandList* commandList, uint32_t rootParameterIndex) {
    commandList->SetGraphicsRootConstantBufferView(rootParameterIndex, constBuffer_->GetGPUVirtualAddress());
}