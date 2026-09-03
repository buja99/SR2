#pragma once
#include "IEnemyState.h"

class EnemyStateIdle : public IEnemyState {
public:
    void Enter(Enemy* enemy) override;
    void Update(Enemy* enemy) override;
    void Exit(Enemy* enemy) override;
private:
    int timer_ = 0; 
};

