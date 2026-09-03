#pragma once
#include <d3d12.h>
#include <cstdint>

class ILight {
public:
    virtual ~ILight() = default;

    // Initialize lighting (create GPU constant buffer)
    virtual void Initialize(ID3D12Device* device) = 0;

    // Update lighting data
    virtual void Update() = 0;

    // Bind light data to the GPU before rendering
    virtual void Bind(ID3D12GraphicsCommandList* commandList, uint32_t rootParameterIndex) = 0;
};
