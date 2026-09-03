#pragma once
#include <vector>
#include <string>
#include <cstdint>
#include "Material.h"
#include "Matrix4x4.h"
#include "Geo.h"
#include "VertexData.h"

struct KeyframeVector3 { Vector3 value;    float time; };
struct KeyframeQuaternion { Quaternion value; float time; };

struct AnimationCurveVector3 { std::vector<KeyframeVector3>    keyframes; };
struct AnimationCurveQuaternion { std::vector<KeyframeQuaternion> keyframes; };

struct AnimationChannel {
	std::string nodeName;
	AnimationCurveVector3    translate;
	AnimationCurveQuaternion rotate;
	AnimationCurveVector3    scale;
};

struct AnimationData {
	std::string name;
	float duration = 0.0f;
	std::vector<AnimationChannel> channels;
};

struct Node {
	Matrix4x4 localMatrix;
	std::string name;
	std::vector<Node> children;
	std::vector<uint32_t> meshIndices;
};

struct SubMesh {
	uint32_t indexStart = 0;   
	uint32_t indexCount = 0;   
	uint32_t materialIndex = 0; 
};

struct ModelData {
	std::vector<VertexDataStatic> vertices;
	std::vector<VertexDataAnimated> verticesAnimated;
	std::vector<uint32_t> indices;
	MaterialData material;
	Node rootNode;
	std::vector<AnimationData> animations;
	std::vector<SubMesh> subMeshes;
};