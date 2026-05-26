#include "PostProcessManager.h"
#include "SrvManager.h"
#include <cassert>

PostProcessManager* PostProcessManager::GetInstance() {
    static PostProcessManager instance;
    return &instance;
}

void PostProcessManager::Initialize(ID3D12Device* device) {
    device_ = device; // 나중에 AddEffect를 할 때 파이프라인을 만들어주기 위해 저장해둡니다.
    /*
    for (int i = 0; i < 2; ++i) {
        // 1. 화면 해상도 크기의 버퍼 텍스처 리소스 생성 (ResourceUtils 등 활용)
        pingPongBuffers_[i] = ResourceUtils::CreateTextureResource(device_, 1280, 720, ...);

        // 2. SrvManager와 RtvManager에 각각 등록하여 인덱스/핸들 획득
        pingPongSRVs_[i] = SrvManager::GetInstance()->AllocateDescriptor(pingPongBuffers_[i].Get());
        pingPongRTVs_[i] = RtvManager::GetInstance()->AllocateDescriptor(pingPongBuffers_[i].Get());
    }
    */
}

void PostProcessManager::Cleanup() {
    // vector가 비워지면서 안에 있는 이펙트들의 메모리도 자동으로 해제
    effects_.clear();
    for (int i = 0; i < 2; ++i) {
        pingPongBuffers_[i].Reset();
    }
}

void PostProcessManager::AddEffect(std::unique_ptr<IPostEffect> effect) {
    if (device_ && effect) {
        // 매니저에 이펙트가 등록되는 순간 파이프라인(Initialize)을 생성
        effect->Initialize(device_);
    }
    effects_.push_back(std::move(effect));
}

void PostProcessManager::ClearEffects() {
    effects_.clear();
}

void PostProcessManager::Draw(ID3D12GraphicsCommandList* commandList, uint32_t offscreenSRVIndex) {
    if (effects_.empty()) {
        return;
    }

    SrvManager::GetInstance()->PreDraw();

    // 1. 첫 번째 이펙트의 입력은 '3D 씬이 그려진 원본 오프스크린 텍스처'로 시작합니다.
    uint32_t currentInputSRV = offscreenSRVIndex;

    // 핑퐁 버퍼의 토글을 관리할 인덱스 (0 또는 1)
    int currentTargetIndex = 0;

    for (size_t i = 0; i < effects_.size(); ++i) {
        auto& effect = effects_[i];
        bool isLast = (i == effects_.size() - 1);

        if (isLast) {
            // 2. ⭐ 마지막 이펙트라면 최종 화면(Backbuffer)에 바로 그려야 합니다.
            // 기존 엔진 구조에서 백버퍼를 RTV로 세팅해 주던 코드를 호출하세요.
            // 예시: DirectXCommon::GetInstance()->SetBackBufferAsRenderTarget(commandList);
        } else {
            // 3. ⭐ 중간 이펙트라면 임시 도화지(핑퐁 버퍼) 중 하나를 RTV로 지정합니다.
            // D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = pingPongRTVs_[currentTargetIndex];
            // commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

            // 임시 도화지 화면을 한 번 깔끔하게 지워줍니다 (ClearRenderTargetView)
            // FLOAT clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
            // commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
        }

        // 4. 현재 지정된 입력 텍스처 번호를 넘겨주며 쉐이더로 화면을 그립니다.
        effect->Draw(commandList, currentInputSRV);

        // 5. 리소스 배리어(Resource Barrier) 전환 필요!
        // 방금 RTV로 썼던 버퍼를 다음 이펙트가 읽을 수 있도록 D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE 상태로 변환해야 합니다.

        if (!isLast) {
            // 6. ⭐ 중요: 다음 이펙트의 입력(SRV)은 '방금 내가 그림을 그린 임시 버퍼'가 됩니다.
            currentInputSRV = pingPongSRVs_[currentTargetIndex];

            // 7. ⭐ 핑퐁 체인지: 다음 도화지는 반대편 버퍼(0번이었다면 1번, 1번이었다면 0번)로 교체합니다.
            currentTargetIndex = 1 - currentTargetIndex;
        }
    }
}