#include "PlayerStateIdle.h"
#include "PlayerStateMove.h"
#include "Player.h"
#include "Input.h"

void PlayerStateIdle::Enter(Player* player) {
   
}

void PlayerStateIdle::Update(Player* player) {
    Input* input = Input::GetInstance();

  
    if (input->PushKey(DIK_LEFT) || input->PushKey(DIK_RIGHT) ||
        input->PushKey(DIK_UP) || input->PushKey(DIK_DOWN)) {

        player->ChangeState(std::make_unique<PlayerStateMove>());
        return;
    }

   
}

void PlayerStateIdle::Exit(Player* player) {}