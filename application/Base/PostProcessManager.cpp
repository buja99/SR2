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
    device_ = device;

    HRESULT hr;

    // SR2님의 해상도 크기에 맞춤 (기본 1280x720 가정)
    const UINT textureWidth = 1280;
    const UINT textureHeight = 720;

    // 1. 렌더 타겟 플래그를 가진 텍스처 리소스 Desc 설정
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
        // 2. 핑퐁 버퍼 텍스처 리소스 생성
        hr = device_->CreateCommittedResource(
            &heapProps, D3D12_HEAP_FLAG_NONE, &resDesc,
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, &clearValue,
            IID_PPV_ARGS(&pingPongBuffers_[i])
        );
        assert(SUCCEEDED(hr));

        // 3. ⭐ [정정] SrvManager 구조에 맞춘 SRV 할당 및 생성
        // SrvManager::Allocate()를 통해 사용 가능한 빈 인덱스를 먼저 받습니다.
        uint32_t srvIndex = SrvManager::GetInstance()->Allocate();

        // SrvManager에 이미 구현되어 있는 Texture2D SRV 생성 함수를 호출합니다!
        SrvManager::GetInstance()->CreatSRVforTexture2D(
            srvIndex,
            pingPongBuffers_[i].Get(),
            DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
            1
        );

        // 생성된 인덱스로부터 진짜 GPU 핸들을 얻어와 멤버 변수에 저장합니다.
        pingPongSRVHandles_[i] = SrvManager::GetInstance()->GetGPUDescriptorHandle(srvIndex);

        // 4. RTV 핸들 생성 및 저장
        // RTV 매니저 클래스가 따로 있다면 그 구조를 쓰시거나, 
        // 현재 구조에서 RTV 힙 핸들을 만드는 방식으로 매핑해 주어야 합니다.
        // pingPongRTVHandles_[i] = RtvManager::GetInstance()->AllocateCpuHandle();
        // device_->CreateRenderTargetView(pingPongBuffers_[i].Get(), nullptr, pingPongRTVHandles_[i]);
    }
}

void PostProcessManager::Cleanup() {
    effects_.clear();
    for (int i = 0; i < kNumPingPongBuffers; ++i) {
        pingPongBuffers_[i].Reset();
    }
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

    // ⭐ [정정] GetGPUHandle 대신 실제 구현된 GetGPUDescriptorHandle 호출!
    D3D12_GPU_DESCRIPTOR_HANDLE currentInputSRV = SrvManager::GetInstance()->GetGPUDescriptorHandle(offscreenSRVIndex);

    int currentTargetIndex = 0;

    for (size_t i = 0; i < effects_.size(); ++i) {
        auto& effect = effects_[i];
        bool isLast = (i == effects_.size() - 1);

        if (isLast) {
            // 마지막 이펙트는 최종 백버퍼 화면에 그립니다.
            DirectXCommon::GetInstance()->SetBackBufferAsRenderTarget();
        } else {
            // 중간 이펙트는 임시 핑퐁 버퍼의 RTV 상태로 전환 후 타겟 설정
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

        // ⭐ 다형성 렌더링 호출
        // (참고: 각 이펙트의 Draw 함수가 uint32_t 인덱스 대신 D3D12_GPU_DESCRIPTOR_HANDLE을 받도록 인터페이스를 고치면 가장 좋습니다)
        // effect->Draw(commandList, currentInputSRV);

        if (!isLast) {
            // 다음 이펙트가 읽을 수 있도록 SRV 상태로 원복
            D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
                pingPongBuffers_[currentTargetIndex].Get(),
                D3D12_RESOURCE_STATE_RENDER_TARGET,
                D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
            );
            commandList->ResourceBarrier(1, &barrier);

            // 입력 핸들을 방금 그린 텍스처의 SRV 핸들로 핑퐁 체인지!
            currentInputSRV = pingPongSRVHandles_[currentTargetIndex];

            // 0 -> 1 -> 0 -> 1 교차 토글
            currentTargetIndex = 1 - currentTargetIndex;
        }
    }
}