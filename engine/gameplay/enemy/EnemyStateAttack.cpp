#include "EnemyStateAttack.h"
#include "Enemy.h"
#include "EnemyStateIdle.h" // 공격이 끝난 후 돌아갈 대기 상태 헤더
#include "Player.h"
#include <memory>

void EnemyStateAttack::Enter(Enemy* enemy) {
    // 1. 타이머 초기화
    attackTimer_ = 0;

    // 2. 공격 시작 연출 처리
    // 예: enemy->PlayAnimation("Attack"); // 공격 애니메이션 재생
    // 예: Sound::GetInstance()->Play("EnemyAttack.wav"); // 공격 효과음

    // 3. 플레이어를 향해 순간적으로 조준 (선택)
    // 만약 공격하는 순간 플레이어가 있는 방향을 딱 쳐다보게 만들고 싶다면 
    // 여기서 방향 벡터를 계산해 적의 Rotation을 세팅해 줄 수 있습니다.
}

void EnemyStateAttack::Update(Enemy* enemy) {
    // 매 프레임 타이머 증가
    attackTimer_++;

    // [중요 로직] 공격 판정 타이밍 제어
    // 애니메이션 프레임 중, 무기를 휘두르는 특정 타이밍(예: 30프레임째)에만 
    // 충돌 판정(OBB)을 활성화하거나 대미지를 주는 플래그를 켤 수 있습니다.
    if (attackTimer_ == 30) {
        // 이 타이밍에 충돌 영역을 활성화하거나 Player에게 데미지를 가함
        // 씬 내부의 충돌 매니저가 처리하게 하거나, 강제로 플레이어 포인터를 받아와 처리할 수 있습니다.
    }

    // ⭐ [상태 전환] 설정한 공격 시간(60프레임)이 모두 끝나면 대기(Idle) 상태로 복귀합니다.
    if (attackTimer_ >= attackDuration_) {
        // 완벽한 스테이트 패턴 순환: Attack ➔ Idle
        enemy->ChangeState(std::make_unique<EnemyStateIdle>());
        return;
    }
}

void EnemyStateAttack::Exit(Enemy* enemy) {
    // 공격 상태를 빠져나갈 때 1회 실행
    // 예: 무기 충돌 판정(Collider) 강제 비활성화, 피격 플래그 리셋 등 안전장치 처리
}