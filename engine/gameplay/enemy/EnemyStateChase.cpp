#include "EnemyStateChase.h"
#include "Enemy.h"
#include "Player.h"
#include "MyMath.h" 
// #include "EnemyStateAttack.h" 

void EnemyStateChase::Enter(Enemy* enemy) {
    // Execute once when entering the state
    // Example: Change the enemy's animation to "Run"
}

void EnemyStateChase::Update(Enemy* enemy) {
   
    Player* player = enemy->GetPlayer();

   
    if (!player) {
        return;
    }

   
    Vector3 enemyPos = enemy->GetWorldTransform().translate_;
    Vector3 playerPos = player->GetPosition();

    
    Vector3 direction = MyMath::Subtract(playerPos, enemyPos);

   
    float distance = MyMath::length(direction);

    
    if (distance <= attackRange_) {
        // Switch to the Attack state when the enemy gets close enough.
        // enemy->ChangeState(std::make_unique<EnemyStateAttack>());
        return; // Exit immediately to prevent further movement logic after the state change.
    }

    
    direction = MyMath::normalize(direction);


    Vector3 velocity = MyMath::Multiply(direction, chaseSpeed_); 
    enemy->GetWorldTransform().translate_ = MyMath::Add(enemyPos, velocity);


}

void EnemyStateChase::Exit(Enemy* enemy) {
    // Execute once when exiting the chase state
    // Example: Disable the running effect
}
