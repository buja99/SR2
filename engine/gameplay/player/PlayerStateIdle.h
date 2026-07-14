#pragma once
#include "IPlayerState.h"


class Player;

class PlayerStateIdle : public IPlayerState {
public:
    PlayerStateIdle() = default;
    ~PlayerStateIdle() override = default;

    void Enter(Player* player) override;
    void Update(Player* player) override;
    void Exit(Player* player) override;
};