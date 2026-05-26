#pragma once

#include "ILight.h"
#include <vector>
#include <memory>

class LightManager {
public:
    static LightManager* GetInstance();

    void Initialize(ID3D12Device* device);

    // 매 프레임 빛의 상태를 갱신합니다 (깜빡임, 이동 등)
    void Update();

    // 렌더링 직전에 모든 빛 데이터를 GPU에 한 번에 묶어줍니다!
    void BindAll(ID3D12GraphicsCommandList* commandList);

    // 빛을 추가할 때, HLSL의 몇 번 파라미터(슬롯)에 넣을지 인덱스를 같이 받습니다.
    void AddLight(std::unique_ptr<ILight> light, uint32_t rootParameterIndex);

    // 씬 전환 등을 위해 모든 빛을 지웁니다.
    void ClearLights();

private:
    LightManager() = default;
    ~LightManager() = default;
    LightManager(const LightManager&) = delete;
    LightManager& operator=(const LightManager&) = delete;

    // 빛 객체와 그 빛이 묶일 인덱스를 함께 저장하는 구조체
    struct LightEntry {
        std::unique_ptr<ILight> light;
        uint32_t rootParameterIndex;
    };

    std::vector<LightEntry> lights_;
    ID3D12Device* device_ = nullptr;
};