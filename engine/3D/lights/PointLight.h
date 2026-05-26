#pragma once
#include "ILight.h"
#include "Lighting.h" 
#include "MyMath.h"
#include <wrl.h>

class PointLight : public ILight {
public:
    void Initialize(ID3D12Device* device) override;
    void Update() override;
    void Bind(ID3D12GraphicsCommandList* commandList, uint32_t rootParameterIndex) override;

    
    void SetColor(const Vector4& color) { cpuData_.color = color; }
    void SetPosition(const Vector3& pos) { cpuData_.position = pos; }
    void SetIntensity(float intensity) { cpuData_.intensity = intensity; }
    void SetRadius(float radius) { cpuData_.radius = radius; }
    void SetDecay(float decay) { cpuData_.decay = decay; }

private:
    Microsoft::WRL::ComPtr<ID3D12Resource> constBuffer_;
    PointLightData* data_ = nullptr;
    PointLightData cpuData_;
};