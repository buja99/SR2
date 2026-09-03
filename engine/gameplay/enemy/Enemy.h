#pragma once
#include "BaseEnemy.h"
#include "ParticleEffectLibrary.h"
#include "IEnemyState.h" 
#include <memory>

class Player;

class Enemy : public BaseEnemy {
public:
    Enemy();
    ~Enemy() override = default;

    void Initialize() override;
    void Update() override;
    void Draw() override;
    void SetDead(bool isDead) { isDead_ = isDead; }

    int GetType() const override;
    void OnHit(float damage) override;
    void SetCamera(Camera* camera);
    WorldTransform& GetWorldTransform() override;

    void ChangeState(std::unique_ptr<IEnemyState> newState);

    void SetPlayer(Player* player) { player_ = player; }
    Player* GetPlayer() const { return player_; }

private:
	std::unique_ptr<Object3d> enemyModel_;
	std::unique_ptr<WorldTransform> enemyWorldTransform_;
    Camera* camera_ = nullptr;
   
    float hp_ = 100.0f;
	bool isDead_ = false;
	bool isHit_ = false;

    std::unique_ptr<IEnemyState> currentState_;

    Player* player_ = nullptr;
};

