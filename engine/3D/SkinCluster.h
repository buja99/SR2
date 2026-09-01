#pragma once
#include <wrl.h>
#include <d3d12.h>
#include "MyMath.h"
#include "DirectXCommon.h"

using Microsoft::WRL::ComPtr;

class Skeleton;

class SkinCluster {

public:
    SkinCluster() = default;
    ~SkinCluster() = default;
    SkinCluster(const SkinCluster&) = delete;
    SkinCluster& operator=(const SkinCluster&) = delete;

    void Initialize(DirectXCommon* dxCommon, size_t boneCount);
    void Update(const Skeleton& skeleton);
	
    D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const;
    ComPtr<ID3D12Resource> GetResource() const;
	
    uint32_t GetBoneCount() const { return boneCount_; }
    const std::vector<Matrix4x4>& GetFinalMatricesCPU() const { return finalMatrices_; }

private:

    ID3D12Device* device = nullptr;
    DirectXCommon* dxCommon_ = nullptr;

    uint32_t boneCount_ = 0;
    
    std::vector<Matrix4x4> finalMatrices_;

   
    Microsoft::WRL::ComPtr<ID3D12Resource> boneMatrixBuffer_;
    Matrix4x4* mappedMatrices_ = nullptr; 

    static ComPtr<ID3D12Resource> CreateUploadBuffer(
        ID3D12Device* device,
        size_t sizeInBytes,
        const wchar_t* name = L"SkinClusterBuffer"
    );

};

