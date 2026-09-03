#pragma once
#include "IEnemyState.h"

class Enemy;

class EnemyStateDead : public IEnemyState {
public:
    void Enter(Enemy* enemy) override;
    void Update(Enemy* enemy) override;
    void Exit(Enemy* enemy) override;

private:
    // Timer for the death sequence
    int deadTimer_ = 0;

    // Total duration of the death sequence
    const int deadDuration_ = 60;
};

