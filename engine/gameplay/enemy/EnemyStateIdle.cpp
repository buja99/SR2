#include "EnemyStateIdle.h"
#include "Enemy.h"
// #include "EnemyStateMove.h" // 

void EnemyStateIdle::Enter(Enemy* enemy) {
    timer_ = 0;
    // (선택) enemy->GetModel()->PlayAnimation("Idle");
}

void EnemyStateIdle::Update(Enemy* enemy) {
    timer_++;

    // 예시: 120프레임(약 2초) 동안 가만히 있다가 이동 상태로 변경!
    if (timer_ > 120) {
        // enemy->ChangeState(std::make_unique<EnemyStateMove>());
    }
}

void EnemyStateIdle::Exit(Enemy* enemy) {
    // 대기 상태가 끝날 때 할 일
}