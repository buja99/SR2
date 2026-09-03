#pragma once

#include "ILight.h"
#include <vector>
#include <memory>

class LightManager {
public:
    static LightManager* GetInstance();

    void Initialize(ID3D12Device* device);

    
    void Update();

    // Bind all light data to the GPU at once just before rendering.
    void BindAll(ID3D12GraphicsCommandList* commandList);

    // When adding a light, also specify the HLSL parameter (slot) index to bind it to.
    void AddLight(std::unique_ptr<ILight> light, uint32_t rootParameterIndex);


    void ClearLights();

private:
    LightManager() = default;
    ~LightManager() = default;
    LightManager(const LightManager&) = delete;
    LightManager& operator=(const LightManager&) = delete;

   
    struct LightEntry {
        std::unique_ptr<ILight> light;
        uint32_t rootParameterIndex;
    };

    std::vector<LightEntry> lights_;
    ID3D12Device* device_ = nullptr;
};