#pragma once
#include "Vector3.h"
#include "Vector4.h"

struct DirectionalLightData
{
	Vector4 color;
	Vector3 direction;
	float intensity;
};

struct PointLightData {
	Vector4 color;
	Vector3 position;
	float intensity;
	float radius;
	float decay;
	float padding[2];
};
struct SpotLightData {
	Vector4 color;      // 16B

	Vector3 position;   // 12B
	float intensity;    // 4B

	Vector3 direction;  // 12B
	float cutoff;       // 4B

	float outerCutoff;  // 4B
	float decay;        // 4B
	float radius;       // 4B
	float padding;      // 4B 
};

struct AmbientLightData {
	Vector4 color;
};

struct AreaLightData {
	Vector4 color;           // 16B

	Vector3 position;        // 12B
	float intensity;         // 4B

	Vector3 right;           // 12B
	float halfWidth;         // 4B

	Vector3 up;              // 12B
	float halfHeight;        // 4B
};