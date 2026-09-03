#pragma once
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"


struct  VertexDataStatic
{
	Vector4 position;
	Vector2 texCoord;
	Vector3 normal;
};

struct VertexDataAnimated {
    Vector4 position;
    Vector2 texCoord;
    Vector3 normal;
    float weight[4];        
    int32_t boneIndices[4]; 
};

struct SpriteVertexData {
	Vector4 position;
	Vector2 texcoord;
};