#include "EnemyStateIdle.h"
#include "Enemy.h"
// #include "EnemyStateMove.h" // 

void EnemyStateIdle::Enter(Enemy* enemy) {
    timer_ = 0;
    
}

void EnemyStateIdle::Update(Enemy* enemy) {
    timer_++;

    
    if (timer_ > 120) {
        // enemy->ChangeState(std::make_unique<EnemyStateMove>());
    }
}

void EnemyStateIdle::Exit(Enemy* enemy) {
    // Handle actions when exiting the Idle state
}