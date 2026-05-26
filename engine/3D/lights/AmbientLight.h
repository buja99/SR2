#pragma once
#include "ILight.h"
#include "Lighting.h"
#include <wrl.h>

class AmbientLight : public ILight {
public:
    void Initialize(ID3D12Device* device) override;
    void Update() override;
    void Bind(ID3D12GraphicsCommandList* commandList, uint32_t rootParameterIndex) override;

    void SetColor(const Vector4& color) { cpuData_.color = color; }
    Vector4 GetColor() const { return cpuData_.color; }

private:
    Microsoft::WRL::ComPtr<ID3D12Resource> constBuffer_;
    AmbientLightData* data_ = nullptr;
    AmbientLightData cpuData_;
};