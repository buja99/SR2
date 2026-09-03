#include "ParticleManager.h"
#include "Logger.h"
#include "StringUtility.h"
#include "StaticModel.h"
#include "ResourceUtils.h"


using namespace Logger;
using namespace StringUtility;

void ParticleManager::Initialize(DirectXCommon* directXCommon, SrvManager* srvManager) {
#ifdef _DEBUG
	
	assert(directXCommon != nullptr);
	assert(srvManager != nullptr);
#endif // _DEBUG

	dxCommon_ = directXCommon;
	srvManager_ = srvManager;

	randomEngine_.seed(std::random_device{}());

	InitializeVertices();

	CreateGraphicsPipeline();

	CreateVertexBuffer();

}

void ParticleManager::Update() {
	if (!camera_) return;

	calculationBillboardMatrix();

	Matrix4x4 cameraMatrix = camera_->GetWorldMatrix();
	Matrix4x4 viewMatrix = MyMath::Inverse(cameraMatrix);
	Matrix4x4 projectionMatrix = camera_->GetProjectionMatrix();
	Matrix4x4 viewProjectionMatrix = MyMath::Multiply(viewMatrix, projectionMatrix);

	constexpr float deltaTime = 1.0f / 60.0f;

	for (auto& [name, group] : particleGroups) {
		
		group.emitter.frequencyTime += deltaTime;
		if (group.emitter.frequencyTime >= group.emitter.frequency) {
			group.emitter.frequencyTime = 0.0f;
			Emit(name, group.emitter, randomEngine_);
		}

		group.particles.remove_if([deltaTime](Particle& p) {
			p.currentTime += deltaTime;
			return p.currentTime >= p.lifeTime;
			});


		uint32_t  i = 0;
		for (auto& p : group.particles) {
			
			p.transform.translate = MyMath::Add(p.transform.translate, p.velocity);

			if (group.mappedInstanceData && i < initialInstanceCount_) {
				
				Matrix4x4 scaleMatrix = MyMath::MakeScaleMatrix(p.transform.scale);

				
				Matrix4x4 scaleRotate;
				if (useBillboard_) {
					scaleRotate = MyMath::Multiply(scaleMatrix, billboardMatrix);
				} else {
					Matrix4x4 rotX = MyMath::MakeRotateXMatrix(p.transform.rotate.x);
					Matrix4x4 rotY = MyMath::MakeRotateYMatrix(p.transform.rotate.y);
					Matrix4x4 rotZ = MyMath::MakeRotateZMatrix(p.transform.rotate.z);
					Matrix4x4 rotationMatrix = MyMath::Multiply(rotZ, MyMath::Multiply(rotY, rotX));
					scaleRotate = MyMath::Multiply(scaleMatrix, rotationMatrix);
				}

				
				Matrix4x4 worldMatrix = scaleRotate;
				worldMatrix.m[3][0] = p.transform.translate.x;
				worldMatrix.m[3][1] = p.transform.translate.y;
				worldMatrix.m[3][2] = p.transform.translate.z;

				float t = p.currentTime / p.lifeTime;
				float alpha = 1.0f - t * t;

				
				Matrix4x4 wvp = MyMath::Multiply(worldMatrix, viewProjectionMatrix);
				group.mappedInstanceData[i].WVP = wvp;
				group.mappedInstanceData[i].World = worldMatrix;

				
				float flicker = 0.9f + 0.2f * std::sin(p.currentTime * 20.0f);
				group.mappedInstanceData[i].color = p.color;
				group.mappedInstanceData[i].color.w *= alpha * flicker;

			}

			++i;
		}

		group.instanceCount = i;
	}
}

void ParticleManager::Draw() {
	auto commandList = dxCommon_->GetCommandList();
	commandList->SetGraphicsRootSignature(rootSignature.Get());
	commandList->SetPipelineState(graphicsPipelineState.Get());

	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);
	for (auto& [name, group] : particleGroups) {

		if (group.instanceCount > 0) {

			
			if (group.instanceCount <= 0 || !group.mappedInstanceData) {
				continue;
			}
			D3D12_GPU_DESCRIPTOR_HANDLE instanceHandle = srvManager_->GetGPUDescriptorHandle(group.instanceSRVIndex);
			commandList->SetGraphicsRootDescriptorTable(0, instanceHandle);

			srvManager_->SetGraphicsRootDesciptorTable(1, group.textureIndex);

			commandList->DrawInstanced(static_cast<UINT>(vertices_.size()), group.instanceCount, 0, 0);
		}
	}
}

void ParticleManager::calculationBillboardMatrix() {
	
	Matrix4x4 backToFrontMatrix = MyMath::MakeRotateYMatrix(std::numbers::pi_v<float>);

	
	Vector3 cameraRotation = camera_->GetRotate();

	
	Matrix4x4 rotateX = MyMath::MakeRotateXMatrix(cameraRotation.x);
	Matrix4x4 rotateY = MyMath::MakeRotateYMatrix(cameraRotation.y);
	Matrix4x4 rotateZ = MyMath::MakeRotateZMatrix(cameraRotation.z);

	Matrix4x4 cameraRotationOnly = MyMath::Multiply(rotateY, MyMath::Multiply(rotateX, rotateZ));

	
	billboardMatrix = MyMath::Multiply(backToFrontMatrix, cameraRotationOnly);

	
	billboardMatrix.m[3][0] = 0.0f;
	billboardMatrix.m[3][1] = 0.0f;
	billboardMatrix.m[3][2] = 0.0f;
}

Particle ParticleManager::MakeRandomParticle(std::mt19937& randomEngine, const Vector3& translate) {
	std::uniform_real_distribution<float> distX(-10.0f, 10.0f);
	std::uniform_real_distribution<float> distY(0.0f, 2.0f);
	std::uniform_real_distribution<float> distZ(-3.0f, 3.0f);
	std::uniform_real_distribution<float> distFallSpeed(0.01f, 0.05f);
	std::uniform_real_distribution<float> distSideDrift(-0.01f, 0.01f);
	std::uniform_real_distribution<float> distScale(0.1f, 0.3f);
	std::uniform_real_distribution<float> distLifetime(3.0f, 6.0f);
	std::uniform_real_distribution<float> distAlpha(0.4f, 0.8f);

	Particle p{};
	p.transform.translate = {
		translate.x + distX(randomEngine),
		translate.y + distY(randomEngine) + 5.0f,
		translate.z + distZ(randomEngine)
	};
	p.velocity = {
		distSideDrift(randomEngine),
		-distFallSpeed(randomEngine),
		distSideDrift(randomEngine)
	};
	float scale = distScale(randomEngine);
	p.transform.scale = { scale, scale, 1.0f };
	p.transform.rotate = { 0.0f, 0.0f, 0.0f };
	p.color = { 0.2f, 0.2f, 0.2f, distAlpha(randomEngine) };
	p.lifeTime = distLifetime(randomEngine);
	p.currentTime = 0.0f;
	return p;
}

Particle ParticleManager::MakeNewParticle(const Vector3& position, const Vector3& velocity, const Vector3& acceleration, const Vector3& scale, const Vector3& rotation, const Vector4& color, float lifeTime) {
	Particle p{};
	p.transform.translate = position;
	p.velocity = velocity;
	p.acceleration = acceleration;
	p.transform.scale = scale;
	p.transform.rotate = rotation;
	p.color = color;
	p.lifeTime = lifeTime;
	p.currentTime = 0.0f;
	return p;
}



std::list<Particle> ParticleManager::Emit(const std::string& groupName, const Emitter& emitter, std::mt19937& randomEngine) {
	std::list<Particle> particles;

	auto it = generators_.find(groupName);
	if (it != generators_.end()) {
		for (uint32_t i = 0; i < emitter.count; ++i) {
			particles.push_back(it->second(randomEngine, emitter.transform.translate));
		}
	} else {
		for (uint32_t i = 0; i < emitter.count; ++i) {
			particles.push_back(MakeRandomParticle(randomEngine, emitter.transform.translate));
		}
	}

	if (particleGroups.contains(groupName)) {
		particleGroups[groupName].particles.insert(
			particleGroups[groupName].particles.end(),
			particles.begin(), particles.end()
		);
	}

	return particles;
}

void ParticleManager::Emit(const std::string& groupName, const Particle& particle) {
	if (particleGroups.contains(groupName)) {
		particleGroups[groupName].particles.push_back(particle);
	}
}



void ParticleManager::CreateParticleGroup(const std::string& name, const std::string& textureFilePath) {
	if (particleGroups.find(name) != particleGroups.end()) {
		
		assert(false && "Particle group with the given name already exists!");
		return; 
	}


	//new ParticleGroup generation
	ParticleGroup newGroup;

	newGroup.textureIndex = TextureManager::GetInstance()->GetTextureIndexByFilepath(textureFilePath);

	const DirectX::TexMetadata& metadata =
		TextureManager::GetInstance()->GetMetaData(textureFilePath);

	newGroup.instanceCount = 0;
	newGroup.mappedInstanceData = nullptr;
	newGroup.emitter.count = 1;
	newGroup.emitter.frequency = 0.2f;
	newGroup.emitter.frequencyTime = 0.0f;

	particleGroups[name] = std::move(newGroup);

	newGroup.instanceBuffer = ResourceUtils::CreateBufferResource(dxCommon_->GetDevice(), sizeof(ParticleForGPU) * kNumMaxInstance);


	HRESULT hr = newGroup.instanceBuffer->Map(0, nullptr, reinterpret_cast<void**>(&newGroup.mappedInstanceData));
	assert(SUCCEEDED(hr));
	


	// SRV (Structured Buffer )
	newGroup.instanceSRVIndex = srvManager_->Allocate() + TextureManager::kSRVIndexTop;
	srvManager_->CreatSRVforStruturedBuffer(
		newGroup.instanceSRVIndex,
		newGroup.instanceBuffer.Get(),
		static_cast<UINT>(initialInstanceCount_),
		sizeof(ParticleForGPU)
	);





	
	particleGroups[name] = std::move(newGroup);

}
void ParticleManager::SetUseRingMesh(bool flag) {
	useRingMesh_ = flag;
	InitializeVertices();
	CreateVertexBuffer();
}
void ParticleManager::SetUseCylinderMesh(bool flag) {
	useRingMesh_ = flag;
	InitializeVertices();
	CreateVertexBuffer();
}
void ParticleManager::SetEmitterPosition(const std::string& name, const Vector3& pos) {
	if (particleGroups.contains(name)) {
		particleGroups[name].emitter.transform.translate = pos;
	}
}

void ParticleManager::SetEmitterFrequency(const std::string& name, float freq) {
	if (particleGroups.contains(name)) {
		particleGroups[name].emitter.frequency = freq;
	}
}

void ParticleManager::SetEmitterCount(const std::string& name, uint32_t count) {
	if (particleGroups.contains(name)) {
		particleGroups[name].emitter.count = count;
	}
}
void ParticleManager::RegisterGenerator(const std::string& name, const ParticleGenerator& generator) {
	generators_[name] = generator;
}


IDxcBlob* ParticleManager::CompileShader(const std::wstring& filePath, const wchar_t* profile, IDxcUtils* dxcUtils, IDxcCompiler3* dxcCompiler, IDxcIncludeHandler* includeHandler) {
	//hlsl
	Log(ConvertString(std::format(L"Begin CompileShader,path:{},profile:{}\n", filePath, profile)));

	IDxcBlobEncoding* shaderSource = nullptr;
	HRESULT hr = dxcUtils->LoadFile(filePath.c_str(), nullptr, &shaderSource);

	assert(SUCCEEDED(hr));

	DxcBuffer shaderSourceBuffer;
	shaderSourceBuffer.Ptr = shaderSource->GetBufferPointer();
	shaderSourceBuffer.Size = shaderSource->GetBufferSize();
	shaderSourceBuffer.Encoding = DXC_CP_UTF8;

	//Compile
	LPCWSTR arguments[] = {
		filePath.c_str(), //Compiler対象のhlalファイル名
		L"-E",L"main",
		L"-T",profile, // ShderProfileの設定
		L"-Zi",L"-Qembed_debug",
		L"-Od",
		L"-Zpr"
	};
	//Shader Compile
	IDxcResult* shaderResult = nullptr;
	hr = dxcCompiler->Compile(
		&shaderSourceBuffer,         //読み込んだfile
		arguments,					 //Compile option
		_countof(arguments),		 //Compile optionの数
		includeHandler,				 //includeが含まれた諸々
		IID_PPV_ARGS(&shaderResult)	 //Compile結果
	);
	assert(SUCCEEDED(hr));
	//警告・エラー
	IDxcBlobUtf8* shaderError = nullptr;
	shaderResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&shaderError), nullptr);
	if (shaderError != nullptr && shaderError->GetStringLength() != 0) {
		Log(shaderError->GetStringPointer());
		assert(false);
	}
	//
	IDxcBlob* shaderBlob = nullptr;
	hr = shaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr);
	assert(SUCCEEDED(hr));
	Log(ConvertString(std::format(L"Compile Succeeded,path:{},profile:{}\n", filePath, profile)));
	shaderSource->Release();
	shaderResult->Release();
	return shaderBlob;
}

void ParticleManager::CreateRootSignature() {
	HRESULT hr;

	//DescriptorRange
	D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
	descriptorRange[0].BaseShaderRegister = 0;
	descriptorRange[0].NumDescriptors = 1;
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_DESCRIPTOR_RANGE descriptorRangeForInstancing[1] = {};
	descriptorRangeForInstancing[0].BaseShaderRegister = 0;
	descriptorRangeForInstancing[0].NumDescriptors = 1;
	descriptorRangeForInstancing[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRangeForInstancing[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	//RootSignature作成
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	//複数設定できるので配列。今回は結果１つだけなので長さ1の配列
	D3D12_ROOT_PARAMETER rootParameters[2] = {};
	//rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; //CBVを使う
	//rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; //PixelShaderで使う
	//rootParameters[0].Descriptor.ShaderRegister = 0; //レジスタ番号0とバインド
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX; ///VertexShaderで使う
	rootParameters[0].DescriptorTable.pDescriptorRanges = descriptorRangeForInstancing;
	rootParameters[0].DescriptorTable.NumDescriptorRanges = _countof(descriptorRangeForInstancing);
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; //DescriptorTableを使う
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; //PixelShaderで使う
	rootParameters[1].DescriptorTable.pDescriptorRanges = descriptorRange;
	rootParameters[1].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);
	//rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; //CBVを使う
	//rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; //PixelShaderで使う
	//rootParameters[3].Descriptor.ShaderRegister = 1; //レジスタ番号1
	descriptionRootSignature.pParameters = rootParameters; //rootParameters配列へのポインタ
	descriptionRootSignature.NumParameters = _countof(rootParameters); //配列の長さ

	//Sampler
	D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
	staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;
	staticSamplers[0].ShaderRegister = 0;
	staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	descriptionRootSignature.pStaticSamplers = staticSamplers;
	descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);

	//シリアライズしてバイナリする
	ComPtr<ID3DBlob> signatureBlob = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	hr = D3D12SerializeRootSignature(&descriptionRootSignature,
		D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	if (FAILED(hr)) {
		Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		assert(false);
	}
	//バイナリを元に生成

	hr = dxCommon_->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(),
		signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
	assert(SUCCEEDED(hr));

	rootSignature->SetName(L"ParticleRootSignature");
}

void ParticleManager::CreateGraphicsPipeline() {
	HRESULT hr;
	//DXC Compiler 初期化
	IDxcUtils* dxcUtils = nullptr;
	IDxcCompiler3* dxcCompiler = nullptr;
	hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils));
	assert(SUCCEEDED(hr));
	hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler));
	assert(SUCCEEDED(hr));
	IDxcIncludeHandler* includeHandler = nullptr;
	hr = dxcUtils->CreateDefaultIncludeHandler(&includeHandler);
	assert(SUCCEEDED(hr));

	//Shader Compile
	ComPtr<IDxcBlob> vertexShaderBlob = CompileShader(L"resources/shaders/Particle.VS.hlsl",
		L"vs_6_0", dxcUtils, dxcCompiler, includeHandler);
	assert(vertexShaderBlob != nullptr);

	ComPtr<IDxcBlob> pixelShaderBlob = CompileShader(L"resources/shaders/Particle.PS.hlsl",
		L"ps_6_0", dxcUtils, dxcCompiler, includeHandler);
	assert(pixelShaderBlob != nullptr);




	// InputLayout
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[2] = {};

	inputElementDescs[0].SemanticName = "POSITION";
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescs[1].SemanticName = "TEXCOORD";
	inputElementDescs[1].SemanticIndex = 0;
	inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = _countof(inputElementDescs);

	//BlendState
	D3D12_BLEND_DESC blendDesc{};
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;

	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;

	//RasterizerState
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
	//rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	CreateRootSignature();

	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
	graphicsPipelineStateDesc.pRootSignature = rootSignature.Get();
	graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;
	graphicsPipelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(),
	vertexShaderBlob->GetBufferSize() };
	graphicsPipelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(),
	pixelShaderBlob->GetBufferSize() };
	graphicsPipelineStateDesc.BlendState = blendDesc;
	graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;

	graphicsPipelineStateDesc.NumRenderTargets = 1;
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

	graphicsPipelineStateDesc.PrimitiveTopologyType =
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	graphicsPipelineStateDesc.SampleDesc.Count = 1;
	graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

	graphicsPipelineStateDesc.DepthStencilState = depthStencilDesc;
	graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	hr = dxCommon_->GetDevice()->CreateGraphicsPipelineState(&graphicsPipelineStateDesc,
		IID_PPV_ARGS(&graphicsPipelineState));
	assert(SUCCEEDED(hr));

}

void ParticleManager::InitializeVertices() {
	vertices_.clear();

	if (useCylinderMesh_) {
		const uint32_t kCylinderDivide = 32;
		const float kTopRadius = 1.0f;
		const float kBottomRadius = 1.0f;
		const float kHeight = 3.0f;

		const float radianPerDivide = 2.0f * std::numbers::pi_v<float> / float(kCylinderDivide);

		for (uint32_t index = 0; index < kCylinderDivide; ++index) {
			float sin = std::sin(index * radianPerDivide);
			float cos = std::cos(index * radianPerDivide);
			float sinNext = std::sin((index + 1) * radianPerDivide);
			float cosNext = std::cos((index + 1) * radianPerDivide);
			float u = float(index) / float(kCylinderDivide);
			float uNext = float(index + 1) / float(kCylinderDivide);

			
			vertices_.push_back({ { sin * kTopRadius, kHeight, cos * kTopRadius, 1.0f }, { u, 0.0f } });
			vertices_.push_back({ { sinNext * kTopRadius, kHeight, cosNext * kTopRadius, 1.0f }, { uNext, 0.0f } });
			vertices_.push_back({ { sin * kBottomRadius, 0.0f, cos * kBottomRadius, 1.0f }, { u, 1.0f } });

			
			vertices_.push_back({ { sin * kBottomRadius, 0.0f, cos * kBottomRadius, 1.0f }, { u, 1.0f } });
			vertices_.push_back({ { sinNext * kTopRadius, kHeight, cosNext * kTopRadius, 1.0f }, { uNext, 0.0f } });
			vertices_.push_back({ { sinNext * kBottomRadius, 0.0f, cosNext * kBottomRadius, 1.0f }, { uNext, 1.0f } });
		}
	} else if (useRingMesh_) {
		
		const uint32_t kRingDivide = 32;
		const float kOuterRadius = 1.0f;
		const float kInnerRadius = 0.6f;
		const float radianPrDivide = 2.0f * std::numbers::pi_v<float> / float(kRingDivide);

		for (uint32_t index = 0; index < kRingDivide; ++index) {
			float sin = std::sin(index * radianPrDivide);
			float cos = std::cos(index * radianPrDivide);
			float sinNext = std::sin((index + 1) * radianPrDivide);
			float cosNext = std::cos((index + 1) * radianPrDivide);
			float u = float(index) / float(kRingDivide);
			float uNext = float(index + 1) / float(kRingDivide);

			
			vertices_.push_back({ { -sin * kOuterRadius, cos * kOuterRadius, 0.0f, 1.0f }, { u, 0.0f } });
			vertices_.push_back({ { -sinNext * kOuterRadius, cosNext * kOuterRadius, 0.0f, 1.0f }, { uNext, 0.0f } });
			vertices_.push_back({ { -sin * kInnerRadius, cos * kInnerRadius, 0.0f, 1.0f }, { u, 1.0f } });

			
			vertices_.push_back({ { -sin * kInnerRadius, cos * kInnerRadius, 0.0f, 1.0f }, { u, 1.0f } });
			vertices_.push_back({ { -sinNext * kOuterRadius, cosNext * kOuterRadius, 0.0f, 1.0f }, { uNext, 0.0f } });
			vertices_.push_back({ { -sinNext * kInnerRadius, cosNext * kInnerRadius, 0.0f, 1.0f }, { uNext, 1.0f } });
		}
	} else {
		
		vertices_.push_back({ { -1.0f,  1.0f, 0.0f, 1.0f }, { 0.0f, 0.0f } }); // 좌상
		vertices_.push_back({ { -1.0f, -1.0f, 0.0f, 1.0f }, { 0.0f, 1.0f } }); // 좌하
		vertices_.push_back({ {  1.0f,  1.0f, 0.0f, 1.0f }, { 1.0f, 0.0f } }); // 우상

		vertices_.push_back({ {  1.0f,  1.0f, 0.0f, 1.0f }, { 1.0f, 0.0f } }); // 우상
		vertices_.push_back({ { -1.0f, -1.0f, 0.0f, 1.0f }, { 0.0f, 1.0f } }); // 좌하
		vertices_.push_back({ {  1.0f, -1.0f, 0.0f, 1.0f }, { 1.0f, 1.0f } }); // 우하
	}
}

void ParticleManager::CreateVertexBuffer() {

	auto device = dxCommon_->GetDevice();

	vertexBufferResource_ = ResourceUtils::CreateBufferResource(device, sizeof(ParticleVertex) * vertices_.size());

	vertexBufferView_.BufferLocation = vertexBufferResource_->GetGPUVirtualAddress();
	vertexBufferView_.SizeInBytes = static_cast<UINT>(sizeof(ParticleVertex) * vertices_.size());
	vertexBufferView_.StrideInBytes = sizeof(ParticleVertex);

	ParticleVertex* vertexDataParticle = nullptr;
	vertexBufferResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexDataParticle));

	memcpy(vertexDataParticle, vertices_.data(), sizeof(ParticleVertex) * vertices_.size());

	vertexBufferResource_->Unmap(0, nullptr);
}