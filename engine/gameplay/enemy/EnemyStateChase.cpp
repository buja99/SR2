#include "EnemyStateChase.h"
#include "Enemy.h"
#include "Player.h"
#include "MyMath.h" 
// #include "EnemyStateAttack.h" // 나중에 공격 상태를 만들면 주석 해제하세요!

void EnemyStateChase::Enter(Enemy* enemy) {
    // 상태 진입 시 1회 실행
    // 예: 적의 애니메이션을 "달리기(Run)"로 변경
}

void EnemyStateChase::Update(Enemy* enemy) {
    // 1. 플레이어 정보 가져오기
    Player* player = enemy->GetPlayer();

    // 방어 코드: 플레이어가 세팅되지 않았다면 아무것도 하지 않음
    if (!player) {
        return;
    }

    // 2. 적과 플레이어의 현재 위치 가져오기
    Vector3 enemyPos = enemy->GetWorldTransform().translate_;
    Vector3 playerPos = player->GetPosition();

    // 3. 방향 벡터 계산 (목표 위치 - 내 위치)
    Vector3 direction = MyMath::Subtract(playerPos, enemyPos);

    // 4. 거리(Length) 계산
    float distance = MyMath::length(direction);

    // 5. 공격 사거리 이내로 들어왔는지 확인
    if (distance <= attackRange_) {
        // [상태 전환] 거리가 가까워지면 '공격' 상태로 변경!
        // enemy->ChangeState(std::make_unique<EnemyStateAttack>());
        return; // 상태가 바뀌었으므로 더 이상 이동 로직을 실행하지 않고 종료
    }

    // 6. 거리가 멀다면 플레이어를 향해 이동
    // 방향 벡터를 정규화(길이를 1로 만듦)
    direction = MyMath::normalize(direction);

    // 정규화된 방향에 속도를 곱하여 이번 프레임의 이동량(Velocity) 계산
    Vector3 velocity = MyMath::Multiply(direction, chaseSpeed_);

    // 적의 위치 갱신 (현재 위치 + 이동량)
    enemy->GetWorldTransform().translate_ = MyMath::Add(enemyPos, velocity);

    // (선택) 적이 플레이어를 바라보도록 회전(Rotation)시키는 코드도 여기에 추가할 수 있습니다.
}

void EnemyStateChase::Exit(Enemy* enemy) {
    // 추적 상태를 빠져나갈 때 1회 실행
    // 예: 달리기 이펙트 끄기
}
