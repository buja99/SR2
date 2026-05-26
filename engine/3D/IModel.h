#pragma once
#include "Matrix4x4.h"
#include <string>

// 전방 선언
class Object3dCommon;
struct TransformationMatrix;

class IModel {
public:
    virtual ~IModel() = default;

    virtual void Initialize(Object3dCommon* object3dCommon, const std::string& directorypath, const std::string& filename) = 0;

    virtual void Update(float deltaTime) = 0;

    virtual void Draw(const Matrix4x4& worldMatrix, const Matrix4x4& viewProj, TransformationMatrix* transformData) = 0;

    virtual void Cleanup() = 0;
};