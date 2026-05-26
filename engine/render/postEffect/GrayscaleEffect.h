#pragma once

#include <d3d12.h>
#include <wrl.h>
#include "IPostEffect.h"

using Microsoft::WRL::ComPtr;

struct GrayscaleSettings {
    float strength;
};

class GrayscaleEffect : public IPostEffect {

public:

    void Initialize(ID3D12Device* device) override; 
    void Draw(ID3D12GraphicsCommandList* commandList, uint32_t srvIndex) override; 

    void SetGrayscaleStrength(float strength) { settings_.strength = strength; }
    float GetGrayscaleStrength() const { return settings_.strength; }

private:
    void InitializePipeline(ID3D12Device* device);

    ComPtr<ID3D12RootSignature> grayscaleRootSignature_;
    ComPtr<ID3D12PipelineState> grayscalePipelineState_;
    ComPtr<ID3D12Resource> grayscaleConstBuffer_;

    GrayscaleSettings settings_ = { 1.0f };
    GrayscaleSettings* mappedData_ = nullptr;

};

