#pragma once
#include "IEnemyState.h"

class Enemy;

class EnemyStateChase : public IEnemyState {
public:
    void Enter(Enemy* enemy) override;
    void Update(Enemy* enemy) override;
    void Exit(Enemy* enemy) override;

private:
    // 추적 속도를 조절하는 변수
    float chaseSpeed_ = 0.2f;

    // 공격 사거리 (이 거리보다 가까워지면 공격 상태로 전환)
    float attackRange_ = 3.0f;
};
