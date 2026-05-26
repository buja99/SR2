#pragma once
#include <d3d12.h>
#include <wrl.h>
#include "IPostEffect.h"
#include "Vector2.h"

using Microsoft::WRL::ComPtr;

struct RadialBlurSettings {
    Vector2 center;     
    float blurWidth;    
    int numSamples;
};

class RadialBlurEffect : public IPostEffect {
public:
    void Initialize(ID3D12Device* device) override;
    void Draw(ID3D12GraphicsCommandList* commandList, uint32_t srvIndex) override;

    void SetCenter(const Vector2& center) { settings_.center = center; }
    void SetBlurWidth(float width) { settings_.blurWidth = width; }
    void SetNumSamples(int samples) { settings_.numSamples = samples; }

    const Vector2& GetCenter() const { return settings_.center; }
    float GetBlurWidth() const { return settings_.blurWidth; }
    int GetNumSamples() const { return settings_.numSamples; }

private:
    void InitializePipeline(ID3D12Device* device);

    ComPtr<ID3D12RootSignature> radialBlurRootSignature_;
    ComPtr<ID3D12PipelineState> radialBlurPipelineState_;
    ComPtr<ID3D12Resource> radialBlurConstBuffer_;

    RadialBlurSettings settings_ = { 0.05f, 10 }; 
    RadialBlurSettings* mappedData_ = nullptr;
};
