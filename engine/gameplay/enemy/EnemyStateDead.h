#pragma once
#include "IEnemyState.h"

class Enemy;

class EnemyStateDead : public IEnemyState {
public:
    void Enter(Enemy* enemy) override;
    void Update(Enemy* enemy) override;
    void Exit(Enemy* enemy) override;

private:
    // 사망 연출을 보여줄 타이머
    int deadTimer_ = 0;

    // 사망 연출이 유지될 총 프레임 (예: 60프레임 = 약 1초)
    const int deadDuration_ = 60;
};

