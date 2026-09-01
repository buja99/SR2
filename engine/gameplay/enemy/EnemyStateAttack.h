#pragma once
#include "IEnemyState.h"


class Enemy;

class EnemyStateAttack : public IEnemyState {
public:
    void Enter(Enemy* enemy) override;
    void Update(Enemy* enemy) override;
    void Exit(Enemy* enemy) override;

private:
    // Timer that counts the frames elapsed since entering the attack state
    int attackTimer_ = 0;

    // Total number of frames for the entire attack action (e.g., 60 frames ≈ 1 second)
    // If the project does not have an animation completion check, use this timer to transition between states.
    const int attackDuration_ = 60;
};
