#include "Object3d.hlsli"

struct TransformationMatrix
{
    float4x4 WVP;
    float4x4 World;
    float4x4 WorldInverseTranspose;
};

// 기존의 월드 변환 행렬 (b0)
ConstantBuffer<TransformationMatrix> gTransformtiomMatrix : register(b0);

// [신규] SkinCluster에서 넘겨줄 뼈대들의 최종 행렬 배열 (t0)
StructuredBuffer<float4x4> gBoneMatrices : register(t0);

// [신규] 애니메이션 모델용 정점 입력 규격
struct VertexShaderInput
{
    float4 position : POSITION0;
    float2 texcoord : TEXCOORD0;
    float3 normal : NORMAL0;
    float4 weight : WEIGHT0; // 어떤 뼈에 영향을 받는지 (가중치)
    int4 boneIndices : BONE_INDICES0; // 영향을 주는 뼈의 번호 (최대 4개)
};

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;

    // 1. 스키닝(Skinning) 행렬 계산
    // 뼈 4개의 행렬을 각 가중치(Weight)만큼 곱해서 하나로 섞습니다.
    float4x4 skinMatrix =
        gBoneMatrices[input.boneIndices.x] * input.weight.x +
        gBoneMatrices[input.boneIndices.y] * input.weight.y +
        gBoneMatrices[input.boneIndices.z] * input.weight.z +
        gBoneMatrices[input.boneIndices.w] * input.weight.w;

    // 2. 스키닝이 적용된 로컬 위치와 법선 계산
    float4 skinnedPosition = mul(input.position, skinMatrix);
    float3 skinnedNormal = mul(input.normal, (float3x3) skinMatrix);

    // 3. 기존과 동일한 월드 변환 및 투영 적용 (기준이 원래 position에서 skinnedPosition으로 바뀜)
    output.position = mul(skinnedPosition, gTransformtiomMatrix.WVP);
    output.texcoord = input.texcoord;
    output.normal = normalize(mul(skinnedNormal, (float3x3) gTransformtiomMatrix.WorldInverseTranspose));
    output.worldPosition = mul(skinnedPosition, gTransformtiomMatrix.World).xyz;

    return output;
}