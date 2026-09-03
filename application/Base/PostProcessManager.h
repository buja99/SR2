#pragma once

#include <d3d12.h>
#include <wrl.h>
#include <vector>
#include <memory>
#include "IPostEffect.h"

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

    void AddEffect(std::unique_ptr<IPostEffect> effect);
    void ClearEffects();
    bool HasAnyEffects() const { return !effects_.empty(); }

    template<typename T>
    T* GetEffect(size_t index) {
        if (index < effects_.size()) {
            return dynamic_cast<T*>(effects_[index].get());
        }
        return nullptr;
    }

private:

    // Hide Constructor and Prevent Copying for Singleton Object Creation
    PostProcessManager() = default;
    ~PostProcessManager() = default;
    PostProcessManager(const PostProcessManager&) = delete;
    PostProcessManager& operator=(const PostProcessManager&) = delete;

    ID3D12Device* device_ = nullptr;
    std::vector<std::unique_ptr<IPostEffect>> effects_;

    static constexpr int kNumPingPongBuffers = 2;
    ComPtr<ID3D12Resource> pingPongBuffers_[kNumPingPongBuffers];

    // Store the SRV handle and index allocated by SrvManager for shader input.
    D3D12_GPU_DESCRIPTOR_HANDLE pingPongSRVHandles_[kNumPingPongBuffers];

    // Store the RTV handles for the render targets (views for each temporary rendering buffer).
    D3D12_CPU_DESCRIPTOR_HANDLE pingPongRTVHandles_[kNumPingPongBuffers];

    bool initialized_ = false;
};

