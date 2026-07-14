#include "PlayerStateIdle.h"
#include "PlayerStateMove.h"
#include "Player.h"
#include "Input.h"

void PlayerStateIdle::Enter(Player* player) {
    // 대기 애니메이션 재생
}

void PlayerStateIdle::Update(Player* player) {
    Input* input = Input::GetInstance();

    // 방향키 입력이 감지되면 '이동' 상태로 변경!
    if (input->PushKey(DIK_LEFT) || input->PushKey(DIK_RIGHT) ||
        input->PushKey(DIK_UP) || input->PushKey(DIK_DOWN)) {

        player->ChangeState(std::make_unique<PlayerStateMove>());
        return;
    }

    // 공격 키가 눌리면 '공격' 상태로 변경 (예: Z키)
    // if (input->TriggerKey(DIK_Z)) { ... }
}

void PlayerStateIdle::Exit(Player* player) {}