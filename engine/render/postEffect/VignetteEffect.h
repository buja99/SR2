#pragma once

#include <d3d12.h>
#include <wrl.h>
#include "IPostEffect.h"

using Microsoft::WRL::ComPtr;

struct VignetteSettings {
	float strength;
};

class VignetteEffect : public IPostEffect {


public:


	void Initialize(ID3D12Device* device) override;
	void Draw(ID3D12GraphicsCommandList* commandList, uint32_t srvIndex) override;


	void SetVignetteStrength(float strength) { settings_.strength = strength; }
	float GetVignetteStrength() const { return settings_.strength; }


private:

	void InitializePipeline(ID3D12Device* device);

	ComPtr<ID3D12RootSignature> vignetteRootSignature_;
	ComPtr<ID3D12PipelineState> vignettePipelineState_;
	ComPtr<ID3D12Resource> vignetteConstBuffer_;

	VignetteSettings settings_ = { 1.0f };
	VignetteSettings* mappedData_ = nullptr;


};

