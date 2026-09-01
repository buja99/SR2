#include "EnemyStateDead.h"
#include "Enemy.h"

void EnemyStateDead::Enter(Enemy* enemy) {
    
    deadTimer_ = 0;

    // Disable collision detection
    // Disable the collider immediately upon entering the death state
       // to prevent the player from hitting or colliding with the dead enemy.

    // Start the death sequence
    // Example: enemy->PlayAnimation("Dead");
    // Example: Sound::GetInstance()->Play("EnemyDie.wav");
}

void EnemyStateDead::Update(Enemy* enemy) {
    if (!enemy) return; 

    deadTimer_++;

    // Gradually sink the enemy into the ground
    Vector3 currentPos = enemy->GetWorldTransform().translate_;
    currentPos.y -= 0.05f; 
    enemy->GetWorldTransform().translate_ = currentPos;

   
    if (deadTimer_ >= deadDuration_) {
       
        enemy->SetDead(true);

       
    }
}

void EnemyStateDead::Exit(Enemy* enemy) {
 
}
