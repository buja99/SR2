#pragma once
#include "DirectXCommon.h"
#include "SrvManager.h"
#include <wrl.h>
#include <random>
#include "VertexData.h"
#include "Material.h"
#include "Transform.h"
#include "MyMath.h"
#include "TextureManager.h"
#include <numbers>
#include <list>
#include "Camera.h"
#include "Sprite.h"
#include "SpriteCommon.h"
#include "MyMath.h"
#include <functional>
#include <unordered_map>

struct ParticleVertex {
    Vector4 position; 
    Vector2 texcoord;
};
struct Particle 
{
    Transform transform;
    Vector3 velocity;
    Vector3 acceleration;
    Vector4 color;
    float lifeTime;
    float currentTime;

};
struct  ParticleForGPU    
{
    Matrix4x4 WVP;
    Matrix4x4 World;
    Vector4 color;
};
struct Emitter         
{
    Transform transform;
    uint32_t count;
    float frequency;
    float frequencyTime;
};
struct ParticleGroup {         
    std::string textureFilePath;
    uint32_t textureIndex;
    std::list<Particle> particles;         
    int instanceSRVIndex;                  
    ComPtr<ID3D12Resource> instanceBuffer; 
    int instanceCount;                     
    ParticleForGPU* mappedInstanceData;    
    Emitter emitter;                       
};

const int32_t initialInstanceCount = 100;

using ParticleGenerator = std::function<Particle(std::mt19937&, const Vector3&)>;

class ParticleManager {
public:
    void Initialize(DirectXCommon* directXCommon, SrvManager* srvManager);
    void Update();
    void Draw();
    void calculationBillboardMatrix();

    Particle MakeRandomParticle(std::mt19937& randomEngine, const Vector3& translate);

    Particle MakeNewParticle(
        const Vector3& position,
        const Vector3& velocity,
        const Vector3& acceleration,
        const Vector3& scale,
        const Vector3& rotation,
        const Vector4& color,
        float lifeTime
    );

    std::list<Particle> Emit(const std::string& groupName, const Emitter& emitter, std::mt19937& randomEngine);

    void Emit(const std::string& groupName, const Particle& particle);

    void CreateParticleGroup(const std::string& name, const std::string& textureFilePath);

    void SetInitialInstanceCount(uint32_t count) { initialInstanceCount_ = count; }

    void SetCamera(Camera* camera) { camera_ = camera; }

    void SetUseBillboard(bool use) { useBillboard_ = use; }
    bool GetUseBillboard() const { return useBillboard_; }

    bool useRingMesh_ = false; 
    bool useCylinderMesh_ = false; 
    void SetUseRingMesh(bool flag);
    void SetUseCylinderMesh(bool flag);


    void SetEmitterPosition(const std::string& name, const Vector3& pos);

    void SetEmitterFrequency(const std::string& name, float freq);
    void SetEmitterCount(const std::string& name, uint32_t count);

    void RegisterGenerator(const std::string& name, const ParticleGenerator& generator);

private:


    IDxcBlob* CompileShader(const std::wstring& filePath, const wchar_t* profile, IDxcUtils* dxcUtils, IDxcCompiler3* dxcCompiler, IDxcIncludeHandler* includeHandler);

    void CreateRootSignature();
    void CreateGraphicsPipeline();
    void InitializeVertices();
    void CreateVertexBuffer();

    ComPtr<ID3D12RootSignature> rootSignature;
    ComPtr<ID3D12PipelineState> graphicsPipelineState;

    DirectXCommon* dxCommon_ = nullptr;
    SrvManager* srvManager_ = nullptr;
    SpriteCommon* spriteCommon_ = nullptr;
    const Camera* camera_ = nullptr;
    std::vector<ParticleVertex> vertices_;
    ComPtr<ID3D12Resource> vertexBufferResource_;
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};
    //uint32_t numInstance = 0;


    std::unordered_map<std::string, ParticleGroup> particleGroups;


    const uint32_t kNumMaxInstance = 100;
    Matrix4x4 billboardMatrix;

    bool useBillboard_ = true;

    const int32_t initialInstanceCount = 100;
    uint32_t initialInstanceCount_ = 100;

    std::mt19937 randomEngine_;

    std::unordered_map<std::string, ParticleGenerator> generators_;

};