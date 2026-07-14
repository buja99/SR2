#include "EnemyStateDead.h"
#include "Enemy.h"

void EnemyStateDead::Enter(Enemy* enemy) {
    // 1. 타이머 초기화
    deadTimer_ = 0;

    // 2. 충돌 판정 끄기
    // 사망 상태에 진입하자마자 플레이어가 시체를 때리거나 부딪히지 않도록
    // 콜라이더를 비활성화하는 로직이 들어가는 것이 좋습니다.

    // 3. 사망 연출 시작
    // 예: enemy->PlayAnimation("Dead"); 
    // 예: Sound::GetInstance()->Play("EnemyDie.wav");
}

void EnemyStateDead::Update(Enemy* enemy) {
    if (!enemy) return; // 방어 코드

    deadTimer_++;

    // [연출 예시] 적이 바닥으로 서서히 가라앉게 만들기
    Vector3 currentPos = enemy->GetWorldTransform().translate_;
    currentPos.y -= 0.05f; // 매 프레임 아래로 조금씩 이동
    enemy->GetWorldTransform().translate_ = currentPos;

    // 설정한 사망 연출 시간이 다 끝나면?
    if (deadTimer_ >= deadDuration_) {
        // ⭐ 적이 완전히 죽었다는 플래그를 켭니다!
        enemy->SetDead(true);

        // (주의: 사망 상태이므로 다른 상태로 ChangeState 하지 않습니다)
    }
}

void EnemyStateDead::Exit(Enemy* enemy) {
    // 적 객체 자체가 소멸될 것이므로 보통 Exit에는 아무것도 적지 않습니다.
}
