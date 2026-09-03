#define NOMINMAX
#include "SkinCluster.h"
#include <cassert>
#include "Skeleton.h"
#include <cstring>
#include <algorithm>
#include <limits>

static inline size_t Align256(size_t sz) {
    return (sz + 255) & ~size_t(255);
}



void SkinCluster::Initialize(DirectXCommon* dxCommon, size_t boneCount) {
    dxCommon_ = dxCommon;
    device = dxCommon_->GetDevice().Get();

    
    const size_t clamped = std::clamp<size_t>(
        boneCount,
        static_cast<size_t>(1),
        static_cast<size_t>(std::numeric_limits<uint32_t>::max())
    );

    boneCount_ = static_cast<uint32_t>(clamped);

    // CPU 
    finalMatrices_.resize(boneCount_, MyMath::MakeIdentity4x4());

    // CBV 
    const size_t rawSize = sizeof(Matrix4x4) * static_cast<size_t>(boneCount_);
    const size_t cbSize = Align256(rawSize);

    
    boneMatrixBuffer_ = CreateUploadBuffer(device, static_cast<UINT64>(cbSize), L"SkinClusterBoneMatrices");
    HRESULT hr = boneMatrixBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&mappedMatrices_));
    assert(SUCCEEDED(hr) && mappedMatrices_);

   
    std::memcpy(mappedMatrices_, finalMatrices_.data(), rawSize);
}

void SkinCluster::Update(const Skeleton& skeleton) {
     
    const size_t n = std::min(finalMatrices_.size(), skeleton.bones.size());
    for (size_t i = 0; i < n; ++i) {
        
        finalMatrices_[i] = MyMath::Multiply(skeleton.bones[i].worldMatrix, skeleton.bones[i].offsetMatrix);
    }

    
    const size_t rawSize = sizeof(Matrix4x4) * n;
    memcpy(mappedMatrices_, finalMatrices_.data(), rawSize);
}



D3D12_GPU_VIRTUAL_ADDRESS SkinCluster::GetGPUVirtualAddress() const {
    return boneMatrixBuffer_ ? boneMatrixBuffer_->GetGPUVirtualAddress() : 0;
}

Microsoft::WRL::ComPtr<ID3D12Resource> SkinCluster::GetResource() const {
    return boneMatrixBuffer_;
}

ComPtr<ID3D12Resource> SkinCluster::CreateUploadBuffer(ID3D12Device* device, size_t sizeInBytes, const wchar_t* name) {
    D3D12_HEAP_PROPERTIES heapProps{};
    heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;

    D3D12_RESOURCE_DESC desc{};
    desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    desc.Width = sizeInBytes;
    desc.Height = 1;
    desc.DepthOrArraySize = 1;
    desc.MipLevels = 1;
    desc.Format = DXGI_FORMAT_UNKNOWN;
    desc.SampleDesc.Count = 1;
    desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

    ComPtr<ID3D12Resource> res;
    HRESULT hr = device->CreateCommittedResource(
        &heapProps, D3D12_HEAP_FLAG_NONE,
        &desc, D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr, IID_PPV_ARGS(&res));
    assert(SUCCEEDED(hr));
    if (SUCCEEDED(hr)) { res->SetName(name); }
    return res;
}
