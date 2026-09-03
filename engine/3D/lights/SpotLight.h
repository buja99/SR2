#pragma once
#include "ILight.h"
#include "Lighting.h" 
#include "MyMath.h"
#include <wrl.h>

class SpotLight : public ILight {
public:
    void Initialize(ID3D12Device* device) override;
    void Update() override;
    void Bind(ID3D12GraphicsCommandList* commandList, uint32_t rootParameterIndex) override;

    
    void SetColor(const Vector4& color) { cpuData_.color = color; }
    void SetPosition(const Vector3& pos) { cpuData_.position = pos; }
    void SetIntensity(float intensity) { cpuData_.intensity = intensity; }
    void SetDirection(const Vector3& dir) { cpuData_.direction = dir; }
    void SetCutoff(float cutoff) { cpuData_.cutoff = cutoff; }
    void SetOuterCutoff(float outerCutoff) { cpuData_.outerCutoff = outerCutoff; }
    void SetRadius(float radius) { cpuData_.radius = radius; }
    void SetDecay(float decay) { cpuData_.decay = decay; }

private:
    Microsoft::WRL::ComPtr<ID3D12Resource> constBuffer_;
    SpotLightData* data_ = nullptr; 
    SpotLightData cpuData_;
};