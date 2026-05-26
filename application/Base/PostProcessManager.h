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

    // 렌더링 (체인에 있는 모든 이펙트 순차적 실행)
    void Draw(ID3D12GraphicsCommandList* commandList, uint32_t offscreenSRVIndex);


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

    PostProcessManager() = default;
    ~PostProcessManager() = default;
    PostProcessManager(const PostProcessManager&) = delete;
    PostProcessManager& operator=(const PostProcessManager&) = delete;

    ID3D12Device* device_ = nullptr; // 이펙트 추가 시 초기화를 위해 디바이스 보관

    // ⭐ 이펙트들을 담아두는 '체인(목록)'
    std::vector<std::unique_ptr<IPostEffect>> effects_;

    Microsoft::WRL::ComPtr<ID3D12Resource> pingPongBuffers_[2];

    // 2. 이 텍스처들에 '그림을 그릴 때' 필요한 RTV(Render Target View) 핸들
    D3D12_CPU_DESCRIPTOR_HANDLE pingPongRTVs_[2];

    // 3. 이 텍스처들을 '다음 이펙트의 쉐이더로 넘겨줄 때' 필요한 SRV 인덱스 (SrvManager 연동용)
    uint32_t pingPongSRVs_[2];
};

