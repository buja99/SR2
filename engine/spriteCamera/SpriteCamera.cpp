#include "SpriteCamera.h"
#include "MyMath.h"

using namespace MyMath;

void SpriteCamera::Initialize(float width, float height, float nearZ, float farZ) {
    viewMatrix_ = MyMath::Identity(); 

    projectionMatrix_ = MyMath::MakeOrthographicMatrix(
        0.0f, width,
        height, 0.0f, 
        nearZ, farZ
    );
}
