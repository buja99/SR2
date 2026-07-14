#pragma once
#include "IPlayerState.h"

class Player;

class PlayerStateMove : public IPlayerState {
public:
    PlayerStateMove() = default;
    ~PlayerStateMove() override = default;

    void Enter(Player* player) override;
    void Update(Player* player) override;
    void Exit(Player* player) override;
};