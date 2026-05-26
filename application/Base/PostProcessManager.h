#pragma once

#include <d3d12.h>
#include <wrl.h>
#include "PostEffect.h"
#include <cstdint>

using Microsoft::WRL::ComPtr;

class PostProcessManager {

public:

	static PostProcessManager* GetInstance();
	void Initialize(ID3D12Device* device);
	void Cleanup();

	void Draw(ID3D12GraphicsCommandList* commandList, uint32_t offscreenSRVIndex);

	// ==========================================
	// [ External Control Interface (for Scene / ImGui) ]
	// ==========================================

	void SetPostEffectMode(PostEffectMode mode) { currentMode_ = mode; }
	PostEffectMode GetPostEffectMode() const { return currentMode_; }

	// --- Grayscale ---
	void SetGrayscaleStrength(float strength);
	float GetGrayscaleStrength() const { return grayscaleSettings_.strength; }

	// --- Vignette ---
	void SetVignetteStrength(float strength);
	float GetVignetteStrength() const { return vignetteSettings_.vignetteStrength; }

	// --- Radial Blur ---
	void SetRadialBlurStrength(float strength);
	float GetRadialBlurStrength() const { return radialBlurSettings_.blurStrength; }

	void SetRadialBlurNumSamples(int samples);
	int GetRadialBlurNumSamples() const { return radialBlurSettings_.numSamples; }

	void SetRadialBlurCenter(float x, float y);
	float GetRadialBlurCenterX() const { return radialBlurSettings_.centerX; }
	float GetRadialBlurCenterY() const { return radialBlurSettings_.centerY; }

	// Helper Function for Checking Feature Enablement
	bool IsGrayscaleEnabled() const { return currentMode_ == PostEffectMode::Grayscale; }
	bool IsVignetteEnabled() const { return currentMode_ == PostEffectMode::Vignette; }
	bool IsRadialBlurEnabled() const { return currentMode_ == PostEffectMode::RadialBlur; }

private:

    // Hide Constructor and Prevent Copying for Singleton Object Creation
    PostProcessManager() = default;
    ~PostProcessManager() = default;
    PostProcessManager(const PostProcessManager&) = delete;
    PostProcessManager& operator=(const PostProcessManager&) = delete;

    // Internal Initialization Helper Function (Creates Pipelines for Each Effect)
    void InitializeGrayscalePipeline(ID3D12Device* device);
    void InitializeVignettePipeline(ID3D12Device* device);
    void InitializeRadialBlurPipeline(ID3D12Device* device);

    // Internal Drawing Helper Function
    void DrawGrayscale(ID3D12GraphicsCommandList* commandList);
    void DrawVignette(ID3D12GraphicsCommandList* commandList);
    void DrawRadialBlur(ID3D12GraphicsCommandList* commandList);

private:
    PostEffectMode currentMode_ = PostEffectMode::None;

    // ==========================================
    // [ Resource and Configuration Data ]
    // ==========================================

    // Grayscale
    ComPtr<ID3D12RootSignature> grayscaleRootSignature_;
    ComPtr<ID3D12PipelineState> grayscalePipelineState_;
    ComPtr<ID3D12Resource> grayscaleConstBuffer_;
    GrayscaleSettings grayscaleSettings_ = { 1.0f };

    // Vignette
    ComPtr<ID3D12RootSignature> vignetteRootSignature_;
    ComPtr<ID3D12PipelineState> vignettePipelineState_;
    ComPtr<ID3D12Resource> vignetteConstBuffer_;
    VignetteSettings vignetteSettings_ = { 1.0f };

    // Radial Blur
    ComPtr<ID3D12RootSignature> radialBlurRootSignature_;
    ComPtr<ID3D12PipelineState> radialBlurPipelineState_;
    ComPtr<ID3D12Resource> radialBlurConstBuffer_;
    RadialBlurSettings radialBlurSettings_{};
};

