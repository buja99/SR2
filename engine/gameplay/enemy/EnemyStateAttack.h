#pragma once
#include "IEnemyState.h"

// 전방 선언
class Enemy;

class EnemyStateAttack : public IEnemyState {
public:
    void Enter(Enemy* enemy) override;
    void Update(Enemy* enemy) override;
    void Exit(Enemy* enemy) override;

private:
    // 공격 상태가 시작된 후 경과된 프레임을 세는 타이머
    int attackTimer_ = 0;

    // 전체 공격 동작에 걸리는 총 프레임 수 (예: 60프레임 = 약 1초)
    // 현재 프로젝트에 애니메이션 완료 체크 기능이 없다면, 이 타이머를 기준으로 상태를 전환합니다.
    const int attackDuration_ = 60;
};
