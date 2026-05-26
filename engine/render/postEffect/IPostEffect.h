#pragma once
#include <d3d12.h>
#include <cstdint>

class IPostEffect {
public:
    virtual ~IPostEffect() = default;

    // Effect Initialization
    virtual void Initialize(ID3D12Device* device) = 0;

    // Process Effect Rendering (Receives Command List and Input Texture)
    // Receives SRV Index as a Parameter for Chain Connection
    virtual void Draw(ID3D12GraphicsCommandList* commandList, uint32_t srvIndex) = 0;
};