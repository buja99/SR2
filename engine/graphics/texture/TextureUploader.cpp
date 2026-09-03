#include "TextureUploader.h"
#include <d3dx12.h>
#include <cassert>

ComPtr<ID3D12Resource> TextureUploader::UploadTexture(
    ComPtr<ID3D12Device> device,
    ID3D12GraphicsCommandList* commandList,
    const DirectX::ScratchImage& mipImages,
    ComPtr<ID3D12Resource>& intermediateOut
) {
    const DirectX::TexMetadata& metadata = mipImages.GetMetadata();

   
    D3D12_RESOURCE_DESC texDesc = {};
    texDesc.Width = static_cast<UINT>(metadata.width);
    texDesc.Height = static_cast<UINT>(metadata.height);
    texDesc.MipLevels = static_cast<UINT16>(metadata.mipLevels);
    texDesc.DepthOrArraySize = static_cast<UINT16>(metadata.arraySize);
    texDesc.Format = metadata.format;
    texDesc.SampleDesc.Count = 1;
    texDesc.Dimension = static_cast<D3D12_RESOURCE_DIMENSION>(metadata.dimension);
    texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    D3D12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

    ComPtr<ID3D12Resource> texture;
    HRESULT hr = device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &texDesc,
        D3D12_RESOURCE_STATE_COPY_DEST, 
        nullptr,
        IID_PPV_ARGS(&texture)
    );
    assert(SUCCEEDED(hr));

    UINT subresourceCount = UINT(mipImages.GetImageCount());


    UINT64 uploadBufferSize = GetRequiredIntermediateSize(texture.Get(), 0, subresourceCount);


    D3D12_RESOURCE_DESC uploadDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);
    D3D12_HEAP_PROPERTIES uploadProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

    hr = device->CreateCommittedResource(
        &uploadProps,
        D3D12_HEAP_FLAG_NONE,
        &uploadDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&intermediateOut)
    );
    assert(SUCCEEDED(hr));


    std::vector<D3D12_SUBRESOURCE_DATA> subresources;
    HRESULT result = DirectX::PrepareUpload(
        device.Get(),
        mipImages.GetImages(),
        mipImages.GetImageCount(),
        mipImages.GetMetadata(),
        subresources
    );

    if (FAILED(result)) {
        OutputDebugStringA(" PrepareUpload failed!\n");
        return nullptr; 
    } else {
        OutputDebugStringA(" PrepareUpload succeeded.\n");
    }


    UpdateSubresources(
        commandList,
        texture.Get(),
        intermediateOut.Get(),
        0, 0,
        static_cast<UINT>(subresources.size()),
        subresources.data()
    );


    auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        texture.Get(),
        D3D12_RESOURCE_STATE_COPY_DEST,
        D3D12_RESOURCE_STATE_GENERIC_READ
    );
    commandList->ResourceBarrier(1, &barrier);

    return texture;
}

UploadResult TextureUploader::UploadAndDescribe(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, const DirectX::ScratchImage& mipImages) {
    UploadResult result{};

    const DirectX::TexMetadata& metadata = mipImages.GetMetadata();
    result.format = metadata.format;
    result.isCubemap = metadata.IsCubemap();


    D3D12_RESOURCE_DESC texDesc = {};
    texDesc.Width = static_cast<UINT>(metadata.width);
    texDesc.Height = static_cast<UINT>(metadata.height);
    texDesc.MipLevels = static_cast<UINT16>(metadata.mipLevels);
    texDesc.DepthOrArraySize = static_cast<UINT16>(metadata.arraySize);
    texDesc.Format = metadata.format;
    texDesc.SampleDesc.Count = 1;
    texDesc.Dimension = static_cast<D3D12_RESOURCE_DIMENSION>(metadata.dimension);
    texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    D3D12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

    HRESULT hr = device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &texDesc,
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(&result.texture)
    );
    assert(SUCCEEDED(hr));


    UINT64 uploadSize = GetRequiredIntermediateSize(result.texture.Get(), 0, static_cast<UINT>(metadata.mipLevels));

    D3D12_RESOURCE_DESC uploadDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadSize);
    D3D12_HEAP_PROPERTIES uploadProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

    hr = device->CreateCommittedResource(
        &uploadProps,
        D3D12_HEAP_FLAG_NONE,
        &uploadDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&result.intermediate)
    );
    assert(SUCCEEDED(hr));


    std::vector<D3D12_SUBRESOURCE_DATA> subresources;
    DirectX::PrepareUpload(device, mipImages.GetImages(), mipImages.GetImageCount(), metadata, subresources);

    UpdateSubresources(
        commandList,
        result.texture.Get(),
        result.intermediate.Get(),
        0, 0,
        static_cast<UINT>(subresources.size()),
        subresources.data()
    );


    auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        result.texture.Get(),
        D3D12_RESOURCE_STATE_COPY_DEST,
        D3D12_RESOURCE_STATE_GENERIC_READ
    );
    commandList->ResourceBarrier(1, &barrier);

    D3D12_SHADER_RESOURCE_VIEW_DESC& desc = result.srvDesc;
    desc.Format = metadata.format;
    desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

    if (metadata.IsCubemap()) {
        desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
        desc.TextureCube.MipLevels = static_cast<UINT>(metadata.mipLevels);
        desc.TextureCube.MostDetailedMip = 0;
        desc.TextureCube.ResourceMinLODClamp = 0.0f;

        OutputDebugStringA("Create a cubemap SRV\n");
    } else {
        desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        desc.Texture2D.MipLevels = static_cast<UINT>(metadata.mipLevels);
        desc.Texture2D.MostDetailedMip = 0;
        desc.Texture2D.PlaneSlice = 0;
        desc.Texture2D.ResourceMinLODClamp = 0.0f;

        OutputDebugStringA("Generates a generic 2D texture SRV\n");
    }

    return result;
}

ComPtr<ID3D12Resource> TextureUploader::UploadAndWait(ComPtr<ID3D12Device> device, ComPtr<ID3D12CommandQueue> commandQueue, const DirectX::ScratchImage& mipImages) {
    ComPtr<ID3D12GraphicsCommandList> commandList;
    ComPtr<ID3D12CommandAllocator> allocator;

    HRESULT hr = device->CreateCommandAllocator(
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        IID_PPV_ARGS(&allocator));
    assert(SUCCEEDED(hr));

    hr = device->CreateCommandList(
        0,
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        allocator.Get(),
        nullptr,
        IID_PPV_ARGS(&commandList));
    assert(SUCCEEDED(hr));


    ComPtr<ID3D12Resource> intermediate;


    ComPtr<ID3D12Resource> texture = UploadTexture(device, commandList.Get(), mipImages, intermediate);

    hr = commandList->Close();
    assert(SUCCEEDED(hr));

    ID3D12CommandList* lists[] = { commandList.Get() };
    commandQueue->ExecuteCommandLists(1, lists);

    ComPtr<ID3D12Fence> fence;
    UINT64 fenceValue = 1;
    hr = device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
    assert(SUCCEEDED(hr));

    hr = commandQueue->Signal(fence.Get(), fenceValue);
    assert(SUCCEEDED(hr));

    if (fence->GetCompletedValue() < fenceValue) {
        HANDLE eventHandle = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        assert(eventHandle != nullptr);
        hr = fence->SetEventOnCompletion(fenceValue, eventHandle);
        assert(SUCCEEDED(hr));
        WaitForSingleObject(eventHandle, INFINITE);
        CloseHandle(eventHandle);
    }

    return texture;
}



