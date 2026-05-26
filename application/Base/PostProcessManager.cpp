#include "PostProcessManager.h"
#include "DirectXCommon.h" 
#include "SrvManager.h"
#include <d3dx12.h>
#include <dxcapi.h>
#include <cassert>

#pragma comment(lib, "dxcompiler.lib")

PostProcessManager* PostProcessManager::GetInstance() {
    static PostProcessManager instance;
    return &instance;
}

void PostProcessManager::Initialize(ID3D12Device* device) {
    assert(device != nullptr);

    InitializeGrayscalePipeline(device);
    InitializeVignettePipeline(device);
    InitializeRadialBlurPipeline(device);
}

void PostProcessManager::Cleanup() {
    grayscaleRootSignature_.Reset();
    grayscalePipelineState_.Reset();
    grayscaleConstBuffer_.Reset();

    vignetteRootSignature_.Reset();
    vignettePipelineState_.Reset();
    vignetteConstBuffer_.Reset();

    radialBlurRootSignature_.Reset();
    radialBlurPipelineState_.Reset();
    radialBlurConstBuffer_.Reset();
}

void PostProcessManager::Draw(ID3D12GraphicsCommandList* commandList, uint32_t offscreenSRVIndex) {
    if (currentMode_ == PostEffectMode::None) {
        return;
    }

    // Set Off-Screen Texture to Descriptor Table via SRV Manager (t0)
    SrvManager::GetInstance()->PreDraw();
    SrvManager::GetInstance()->SetGraphicsRootDesciptorTable(0, offscreenSRVIndex);

    switch (currentMode_) {
    case PostEffectMode::Grayscale:
        DrawGrayscale(commandList);
        break;
    case PostEffectMode::Vignette:
        DrawVignette(commandList);
        break;
    case PostEffectMode::RadialBlur:
        DrawRadialBlur(commandList);
        break;
    default:
        break;
    }
}



// ==========================================
// [ Grayscale ]
// ==========================================

void PostProcessManager::SetGrayscaleStrength(float strength) {
    grayscaleSettings_.strength = strength;
    void* mapped = nullptr;
    if (SUCCEEDED(grayscaleConstBuffer_->Map(0, nullptr, &mapped))) {
        memcpy(mapped, &grayscaleSettings_, sizeof(GrayscaleSettings));
        grayscaleConstBuffer_->Unmap(0, nullptr);
    }
}

void PostProcessManager::InitializeGrayscalePipeline(ID3D12Device* device) {
    HRESULT hr;
    ComPtr<IDxcUtils> dxcUtils;
    ComPtr<IDxcCompiler3> dxcCompiler;
    hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils));
    assert(SUCCEEDED(hr));
    hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler));
    assert(SUCCEEDED(hr));
    ComPtr<IDxcIncludeHandler> includeHandler;
    dxcUtils->CreateDefaultIncludeHandler(&includeHandler);

    auto vs = DirectXCommon::GetInstance()->CompileShader(L"Resources/shaders/Grayscale.VS.hlsl", L"vs_6_0", dxcUtils.Get(), dxcCompiler.Get(), includeHandler.Get());
    auto ps = DirectXCommon::GetInstance()->CompileShader(L"Resources/shaders/Grayscale.PS.hlsl", L"ps_6_0", dxcUtils.Get(), dxcCompiler.Get(), includeHandler.Get());

    CD3DX12_DESCRIPTOR_RANGE range;
    range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0); // t0

    CD3DX12_ROOT_PARAMETER params[2];
    params[0].InitAsDescriptorTable(1, &range, D3D12_SHADER_VISIBILITY_PIXEL);
    params[1].InitAsConstantBufferView(0); // b0

    CD3DX12_STATIC_SAMPLER_DESC sampler(0, D3D12_FILTER_MIN_MAG_MIP_LINEAR);

    CD3DX12_ROOT_SIGNATURE_DESC rsDesc;
    rsDesc.Init(_countof(params), params, 1, &sampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    ComPtr<ID3DBlob> sigBlob, errBlob;
    hr = D3D12SerializeRootSignature(&rsDesc, D3D_ROOT_SIGNATURE_VERSION_1, &sigBlob, &errBlob);
    assert(SUCCEEDED(hr));
    hr = device->CreateRootSignature(0, sigBlob->GetBufferPointer(), sigBlob->GetBufferSize(), IID_PPV_ARGS(&grayscaleRootSignature_));
    assert(SUCCEEDED(hr));

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
    psoDesc.pRootSignature = grayscaleRootSignature_.Get();
    psoDesc.VS = { vs->GetBufferPointer(), vs->GetBufferSize() };
    psoDesc.PS = { ps->GetBufferPointer(), ps->GetBufferSize() };
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState.DepthEnable = FALSE;
    psoDesc.DepthStencilState.StencilEnable = FALSE;
    psoDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
    psoDesc.InputLayout = { nullptr, 0 };
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    psoDesc.SampleDesc.Count = 1;

    hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&grayscalePipelineState_));
    assert(SUCCEEDED(hr));

    D3D12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    D3D12_RESOURCE_DESC resDesc = CD3DX12_RESOURCE_DESC::Buffer((sizeof(GrayscaleSettings) + 255) & ~255);

    hr = device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &resDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&grayscaleConstBuffer_));
    assert(SUCCEEDED(hr));

    SetGrayscaleStrength(1.0f);
}

void PostProcessManager::DrawGrayscale(ID3D12GraphicsCommandList* commandList) {
    commandList->SetGraphicsRootSignature(grayscaleRootSignature_.Get());
    commandList->SetPipelineState(grayscalePipelineState_.Get());
    commandList->SetGraphicsRootConstantBufferView(1, grayscaleConstBuffer_->GetGPUVirtualAddress());
    commandList->DrawInstanced(3, 1, 0, 0);
}

// ==========================================
// [ Vignette ]
// ==========================================

void PostProcessManager::SetVignetteStrength(float strength) {
    vignetteSettings_.vignetteStrength = strength;
    void* mapped = nullptr;
    if (SUCCEEDED(vignetteConstBuffer_->Map(0, nullptr, &mapped))) {
        memcpy(mapped, &vignetteSettings_, sizeof(VignetteSettings));
        vignetteConstBuffer_->Unmap(0, nullptr);
    }
}

void PostProcessManager::InitializeVignettePipeline(ID3D12Device* device) {
    HRESULT hr;
    ComPtr<IDxcUtils> dxcUtils;
    ComPtr<IDxcCompiler3> dxcCompiler;
    hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils));
    hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler));
    ComPtr<IDxcIncludeHandler> includeHandler;
    dxcUtils->CreateDefaultIncludeHandler(&includeHandler);

    auto vs = DirectXCommon::GetInstance()->CompileShader(L"Resources/shaders/Vignette.VS.hlsl", L"vs_6_0", dxcUtils.Get(), dxcCompiler.Get(), includeHandler.Get());
    auto ps = DirectXCommon::GetInstance()->CompileShader(L"Resources/shaders/Vignette.PS.hlsl", L"ps_6_0", dxcUtils.Get(), dxcCompiler.Get(), includeHandler.Get());

    CD3DX12_DESCRIPTOR_RANGE range;
    range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

    CD3DX12_ROOT_PARAMETER params[2];
    params[0].InitAsDescriptorTable(1, &range, D3D12_SHADER_VISIBILITY_PIXEL);
    params[1].InitAsConstantBufferView(0);

    CD3DX12_STATIC_SAMPLER_DESC sampler(0, D3D12_FILTER_MIN_MAG_MIP_LINEAR);

    CD3DX12_ROOT_SIGNATURE_DESC rsDesc;
    rsDesc.Init(_countof(params), params, 1, &sampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    ComPtr<ID3DBlob> sigBlob, errBlob;
    hr = D3D12SerializeRootSignature(&rsDesc, D3D_ROOT_SIGNATURE_VERSION_1, &sigBlob, &errBlob);
    hr = device->CreateRootSignature(0, sigBlob->GetBufferPointer(), sigBlob->GetBufferSize(), IID_PPV_ARGS(&vignetteRootSignature_));

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
    psoDesc.pRootSignature = vignetteRootSignature_.Get();
    psoDesc.VS = { vs->GetBufferPointer(), vs->GetBufferSize() };
    psoDesc.PS = { ps->GetBufferPointer(), ps->GetBufferSize() };
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState.DepthEnable = FALSE;
    psoDesc.DepthStencilState.StencilEnable = FALSE;
    psoDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
    psoDesc.InputLayout = { nullptr, 0 };
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    psoDesc.SampleDesc.Count = 1;

    hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&vignettePipelineState_));

    D3D12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    D3D12_RESOURCE_DESC resDesc = CD3DX12_RESOURCE_DESC::Buffer((sizeof(VignetteSettings) + 255) & ~255);

    hr = device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &resDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&vignetteConstBuffer_));

    SetVignetteStrength(1.0f);
}

void PostProcessManager::DrawVignette(ID3D12GraphicsCommandList* commandList) {
    commandList->SetGraphicsRootSignature(vignetteRootSignature_.Get());
    commandList->SetPipelineState(vignettePipelineState_.Get());
    commandList->SetGraphicsRootConstantBufferView(1, vignetteConstBuffer_->GetGPUVirtualAddress());
    commandList->DrawInstanced(3, 1, 0, 0);
}

// ==========================================
// [ Radial Blur ]
// ==========================================

void PostProcessManager::SetRadialBlurStrength(float strength) {
    radialBlurSettings_.blurStrength = strength;
    void* mapped = nullptr;
    if (SUCCEEDED(radialBlurConstBuffer_->Map(0, nullptr, &mapped))) {
        memcpy(mapped, &radialBlurSettings_, sizeof(RadialBlurSettings));
        radialBlurConstBuffer_->Unmap(0, nullptr);
    }
}

void PostProcessManager::SetRadialBlurNumSamples(int samples) {
    radialBlurSettings_.numSamples = samples;
    SetRadialBlurStrength(radialBlurSettings_.blurStrength); // Update buffer
}

void PostProcessManager::SetRadialBlurCenter(float x, float y) {
    radialBlurSettings_.centerX = x;
    radialBlurSettings_.centerY = y;
    SetRadialBlurStrength(radialBlurSettings_.blurStrength); // Update buffer
}

void PostProcessManager::InitializeRadialBlurPipeline(ID3D12Device* device) {
    HRESULT hr;
    ComPtr<IDxcUtils> dxcUtils;
    ComPtr<IDxcCompiler3> dxcCompiler;
    hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils));
    hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler));
    ComPtr<IDxcIncludeHandler> includeHandler;
    dxcUtils->CreateDefaultIncludeHandler(&includeHandler);

    auto vs = DirectXCommon::GetInstance()->CompileShader(L"Resources/shaders/CopyImage.VS.hlsl", L"vs_6_0", dxcUtils.Get(), dxcCompiler.Get(), includeHandler.Get());
    auto ps = DirectXCommon::GetInstance()->CompileShader(L"Resources/shaders/RadialBlur.PS.hlsl", L"ps_6_0", dxcUtils.Get(), dxcCompiler.Get(), includeHandler.Get());

    CD3DX12_DESCRIPTOR_RANGE range;
    range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

    CD3DX12_ROOT_PARAMETER params[2];
    params[0].InitAsDescriptorTable(1, &range, D3D12_SHADER_VISIBILITY_PIXEL);
    params[1].InitAsConstantBufferView(0);

    CD3DX12_STATIC_SAMPLER_DESC sampler(0, D3D12_FILTER_MIN_MAG_MIP_LINEAR);

    CD3DX12_ROOT_SIGNATURE_DESC rsDesc;
    rsDesc.Init(_countof(params), params, 1, &sampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    ComPtr<ID3DBlob> sigBlob, errBlob;
    hr = D3D12SerializeRootSignature(&rsDesc, D3D_ROOT_SIGNATURE_VERSION_1, &sigBlob, &errBlob);
    hr = device->CreateRootSignature(0, sigBlob->GetBufferPointer(), sigBlob->GetBufferSize(), IID_PPV_ARGS(&radialBlurRootSignature_));

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
    psoDesc.pRootSignature = radialBlurRootSignature_.Get();
    psoDesc.VS = { vs->GetBufferPointer(), vs->GetBufferSize() };
    psoDesc.PS = { ps->GetBufferPointer(), ps->GetBufferSize() };
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState.DepthEnable = FALSE;
    psoDesc.DepthStencilState.StencilEnable = FALSE;
    psoDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
    psoDesc.InputLayout = { nullptr, 0 };
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    psoDesc.SampleDesc.Count = 1;
    hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&radialBlurPipelineState_));

    D3D12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    D3D12_RESOURCE_DESC resDesc = CD3DX12_RESOURCE_DESC::Buffer((sizeof(RadialBlurSettings) + 255) & ~255);
    hr = device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &resDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&radialBlurConstBuffer_));

    radialBlurSettings_.centerX = 0.5f;
    radialBlurSettings_.centerY = 0.5f;
    radialBlurSettings_.numSamples = 8;
    SetRadialBlurStrength(0.5f);
}

void PostProcessManager::DrawRadialBlur(ID3D12GraphicsCommandList* commandList) {
    commandList->SetGraphicsRootSignature(radialBlurRootSignature_.Get());
    commandList->SetPipelineState(radialBlurPipelineState_.Get());
    commandList->SetGraphicsRootConstantBufferView(1, radialBlurConstBuffer_->GetGPUVirtualAddress());
    commandList->DrawInstanced(3, 1, 0, 0);
}