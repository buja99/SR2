#pragma once
class Enemy;

class IEnemyState {
public:
    virtual ~IEnemyState() = default;

    // 상태가 시작될 때 1번 호출 (예: 애니메이션 재생)
    virtual void Enter(Enemy* enemy) = 0;

    // 매 프레임 호출 (상태의 실제 로직)
    virtual void Update(Enemy* enemy) = 0;

    // 상태가 끝날 때 1번 호출 (예: 이펙트 끄기)
    virtual void Exit(Enemy* enemy) = 0;
};