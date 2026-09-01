#pragma once
#include "IEnemyState.h"

class Enemy;

class EnemyStateChase : public IEnemyState {
public:
    void Enter(Enemy* enemy) override;
    void Update(Enemy* enemy) override;
    void Exit(Enemy* enemy) override;

private:
    // Variable that controls the chase speed
    float chaseSpeed_ = 0.2f;

    // Attack range (switch to the Attack state when the player is within this distance)
    float attackRange_ = 3.0f;
};
