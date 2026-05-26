#pragma once
#include "ILight.h"
#include "Lighting.h" 
#include "MyMath.h"
#include <wrl.h>

class DirectionalLight : public ILight {
public:
    void Initialize(ID3D12Device* device) override;
    void Update() override;
    void Bind(ID3D12GraphicsCommandList* commandList, uint32_t rootParameterIndex) override;

    // 빛 속성을 외부에서 쉽게 조절하기 위한 Setter
    void SetColor(const Vector4& color) { cpuData_.color = color; }
    void SetDirection(const Vector3& dir) { cpuData_.direction = dir; }
    void SetIntensity(float intensity) { cpuData_.intensity = intensity; }

private:
    Microsoft::WRL::ComPtr<ID3D12Resource> constBuffer_;
    DirectionalLightData* data_ = nullptr; // 전역 네임스페이스의 기존 구조체 참조
    DirectionalLightData cpuData_;
};

