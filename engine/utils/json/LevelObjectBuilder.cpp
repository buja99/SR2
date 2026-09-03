#include "LevelObjectBuilder.h"
#include "ModelManager.h"
#include <iostream>
#include "Object3dCommon.h"
#include "DirectXCommon.h"
#include "Object3d.h"

void LevelObjectBuilder::BuildFromJson(
    const LevelData* levelData,
    std::vector<std::unique_ptr<Object3d>>& objects,
    std::vector<std::unique_ptr<WorldTransform>>& transforms,
    Camera* camera)
{
    assert(levelData);

    
    for (const auto& obj : levelData->objects) {
        if (obj.disabled) {
            continue;
        }

        auto transform = std::make_unique<WorldTransform>();
        transform->Initialize();
        transform->translate_ = obj.translation;
        transform->rotate_ = obj.rotation;
        transform->scale_ = obj.scaling;

        auto model = std::make_unique<Object3d>();
        model->Initialize(Object3dCommon::GetInstance(), transform.get());
        model->SetModel(obj.fileName);
        if (camera) model->SetCamera(camera);

        transforms.push_back(std::move(transform));
        objects.push_back(std::move(model));
    }

    
    for (const auto& enemy : levelData->enemies) {
        auto transform = std::make_unique<WorldTransform>();
        transform->Initialize();
        transform->translate_ = enemy.translation;
        transform->rotate_ = enemy.rotation;
        transform->scale_ = { 1.0f, 1.0f, 1.0f };  

        auto model = std::make_unique<Object3d>();
        model->Initialize(Object3dCommon::GetInstance(), transform.get());
        model->SetModel(enemy.fileName); 
        if (camera) model->SetCamera(camera);

        transforms.push_back(std::move(transform));
        objects.push_back(std::move(model));
    }

    for (const auto& player : levelData->players) {
        auto transform = std::make_unique<WorldTransform>();
        transform->Initialize();
        transform->translate_ = player.translation;
        transform->rotate_ = player.rotation;
        transform->scale_ = { 1.0f, 1.0f, 1.0f };

        auto model = std::make_unique<Object3d>();
        model->Initialize(Object3dCommon::GetInstance(), transform.get());
        model->SetModel("player"); 
        if (camera) model->SetCamera(camera);

        transforms.push_back(std::move(transform));
        objects.push_back(std::move(model));
    }
}
