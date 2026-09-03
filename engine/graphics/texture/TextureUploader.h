#include <wrl.h>
#include <d3d12.h>
#include <DirectXTex.h>
#pragma once

using Microsoft::WRL::ComPtr;


struct UploadResult {
    ComPtr<ID3D12Resource> texture;               
    ComPtr<ID3D12Resource> intermediate;          
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;      
    DXGI_FORMAT format;                           
    bool isCubemap = false;                       
};

class TextureUploader {
public:
    static ComPtr<ID3D12Resource> UploadTexture(
        ComPtr<ID3D12Device> device,
        ID3D12GraphicsCommandList* commandList,
        const DirectX::ScratchImage& mipImages,
        ComPtr<ID3D12Resource>& intermediateBufferOut 
    );
    


    static UploadResult UploadAndDescribe(
        ID3D12Device* device,
        ID3D12GraphicsCommandList* commandList,
        const DirectX::ScratchImage& mipImages
    );

    static ComPtr<ID3D12Resource> UploadAndWait(
        ComPtr<ID3D12Device> device,
        ComPtr<ID3D12CommandQueue> commandQueue,
        const DirectX::ScratchImage& mipImages
    );
};