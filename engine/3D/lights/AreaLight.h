#pragma once
#include "ILight.h"
#include "Lighting.h"
#include "MyMath.h"
#include <wrl.h>

class AreaLight : public ILight {
public:
    void Initialize(ID3D12Device* device) override;
    void Update() override;
    void Bind(ID3D12GraphicsCommandList* commandList, uint32_t rootParameterIndex) override;

    // Setter
    void SetColor(const Vector4& color) { cpuData_.color = color; }
    void SetPosition(const Vector3& pos) { cpuData_.position = pos; }
    void SetIntensity(float intensity) { cpuData_.intensity = intensity; }
    void SetRight(const Vector3& right) { cpuData_.right = right; }
    void SetHalfWidth(float halfWidth) { cpuData_.halfWidth = halfWidth; }
    void SetUp(const Vector3& up) { cpuData_.up = up; }
    void SetHalfHeight(float halfHeight) { cpuData_.halfHeight = halfHeight; }

private:
    Microsoft::WRL::ComPtr<ID3D12Resource> constBuffer_;
    AreaLightData* data_ = nullptr;
    AreaLightData cpuData_;
};