#include "PostProcessManager.h"
#include "SrvManager.h"
#include "DirectXCommon.h"
#include <cassert>
#include <d3dx12.h>

PostProcessManager* PostProcessManager::GetInstance() {
    static PostProcessManager instance;
    return &instance;
}

void PostProcessManager::Initialize(ID3D12Device* device) {

    if (initialized_) {
        return;
    }
    initialized_ = true;
    device_ = device;

    HRESULT hr;

    // Match the resolution size (assuming a default resolution of 1280x720)
    const UINT textureWidth = 1280;
    const UINT textureHeight = 720;

    // 1. Configure the texture resource description with the render target flag
    D3D12_RESOURCE_DESC resDesc = CD3DX12_RESOURCE_DESC::Tex2D(
        DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
        textureWidth, textureHeight,
        1, 1, 1, 0,
        D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET
    );

    D3D12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

    D3D12_CLEAR_VALUE clearValue{};
    clearValue.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    clearValue.Color[0] = 0.0f; clearValue.Color[1] = 0.0f; clearValue.Color[2] = 0.0f; clearValue.Color[3] = 1.0f;

    for (int i = 0; i < kNumPingPongBuffers; ++i) {
        // 2. Create the ping-pong buffer texture resources
        hr = device_->CreateCommittedResource(
            &heapProps, D3D12_HEAP_FLAG_NONE, &resDesc,
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, &clearValue,
            IID_PPV_ARGS(&pingPongBuffers_[i])
        );
        assert(SUCCEEDED(hr));

        // 3. Allocate and create SRVs using the SrvManager
        // Get an available SRV index using SrvManager::Allocate().
        uint32_t srvIndex = SrvManager::GetInstance()->Allocate();

        // Call the Texture2D SRV creation function provided by SrvManager.
        SrvManager::GetInstance()->CreatSRVforTexture2D(
            srvIndex,
            pingPongBuffers_[i].Get(),
            DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
            1
        );

        // Get the GPU handle from the allocated index and store it in the member variable.
        pingPongSRVHandles_[i] = SrvManager::GetInstance()->GetGPUDescriptorHandle(srvIndex);

        // pingPongRTVHandles_[i] = RtvManager::GetInstance()->AllocateCpuHandle();
        // device_->CreateRenderTargetView(pingPongBuffers_[i].Get(), nullptr, pingPongRTVHandles_[i]);
    }
}

void PostProcessManager::Cleanup() {
    effects_.clear();
    for (int i = 0; i < kNumPingPongBuffers; ++i) {
        pingPongBuffers_[i].Reset();
    }
    initialized_ = false;
}

void PostProcessManager::AddEffect(std::unique_ptr<IPostEffect> effect) {
    if (device_ && effect) {
        effect->Initialize(device_);
    }
    effects_.push_back(std::move(effect));
}

void PostProcessManager::ClearEffects() {
    effects_.clear();
}

void PostProcessManager::Draw(ID3D12GraphicsCommandList* commandList, uint32_t offscreenSRVIndex) {
    if (effects_.empty()) return;

    SrvManager::GetInstance()->PreDraw();

    // Call the actual GetGPUDescriptorHandle function instead of GetGPUHandle.
    D3D12_GPU_DESCRIPTOR_HANDLE currentInputSRV = SrvManager::GetInstance()->GetGPUDescriptorHandle(offscreenSRVIndex);

    int currentTargetIndex = 0;

    for (size_t i = 0; i < effects_.size(); ++i) {
        auto& effect = effects_[i];
        bool isLast = (i == effects_.size() - 1);

        if (isLast) {
            DirectXCommon::GetInstance()->SetBackBufferAsRenderTarget();
        } else {
            // For intermediate effects, transition the temporary ping-pong buffer to the render target state and set it as the render target.
            D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
                pingPongBuffers_[currentTargetIndex].Get(),
                D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
                D3D12_RESOURCE_STATE_RENDER_TARGET
            );
            commandList->ResourceBarrier(1, &barrier);

            D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = pingPongRTVHandles_[currentTargetIndex];
            commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

            FLOAT clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
            commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
        }


        if (!isLast) {
            // Transition back to the SRV state so the next effect can read from it.
            D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
                pingPongBuffers_[currentTargetIndex].Get(),
                D3D12_RESOURCE_STATE_RENDER_TARGET,
                D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
            );
            commandList->ResourceBarrier(1, &barrier);

            // Set the input handle to the SRV handle of the texture just rendered.
            currentInputSRV = pingPongSRVHandles_[currentTargetIndex];

            // Toggle the ping-pong buffer alternately: 0 -> 1 -> 0 -> 1.
            currentTargetIndex = 1 - currentTargetIndex;
        }
    }
}