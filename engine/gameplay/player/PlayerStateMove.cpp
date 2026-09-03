#include "PlayerStateMove.h"
#include "PlayerStateIdle.h"
#include "Player.h"
#include "Input.h"
#include "MyMath.h"

void PlayerStateMove::Enter(Player* player) {
   
}

void PlayerStateMove::Update(Player* player) {
    Input* input = Input::GetInstance();
    Vector3 move = { 0.0f, 0.0f, 0.0f };
    float speed = 0.5f;

  
    if (input->PushKey(DIK_LEFT))  move.x -= 1.0f;
    if (input->PushKey(DIK_RIGHT)) move.x += 1.0f;
    if (input->PushKey(DIK_UP))    move.z += 1.0f;
    if (input->PushKey(DIK_DOWN))  move.z -= 1.0f;

   
    if (move.x == 0.0f && move.z == 0.0f) {
        player->ChangeState(std::make_unique<PlayerStateIdle>());
        return;
    }

  
    move = MyMath::normalize(move);
    move = MyMath::Multiply(move, speed);

    WorldTransform* bodyTransform = player->GetBodyTransform();
    bodyTransform->translate_ = MyMath::Add(bodyTransform->translate_, move);
}

void PlayerStateMove::Exit(Player* player) {}