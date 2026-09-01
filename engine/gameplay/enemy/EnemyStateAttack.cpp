#include "EnemyStateAttack.h"
#include "Enemy.h"
#include "EnemyStateIdle.h" 
#include "Player.h"
#include <memory>

void EnemyStateAttack::Enter(Enemy* enemy) {
    // 1. Initialize the timer
    attackTimer_ = 0;

    // 2. Handle the attack start presentation
    // Example: enemy->PlayAnimation("Attack"); // Play the attack animation
    // Example: Sound::GetInstance()->Play("EnemyAttack.wav"); // Play the attack sound effect

    // 3. Instantly aim at the player (optional)
    // If you want the enemy to face the player's direction at the moment of attack,
    // calculate the direction vector here and set the enemy's rotation.
}

void EnemyStateAttack::Update(Enemy* enemy) {
    // Increment the timer every frame
    attackTimer_++;

    // Control the attack hit timing
    // Enable collision detection (OBB) or the damage flag only at a specific point
    // in the attack animation, such as the frame when the weapon is swung.
    if (attackTimer_ == 30) {
        // Enable the collision area or deal damage to the player at this timing.
        // This can be handled by the scene's collision manager or directly using a player pointer.
    }

    // Return to the Idle state when the specified attack duration (60 frames) is complete.
    if (attackTimer_ >= attackDuration_) {
        
        enemy->ChangeState(std::make_unique<EnemyStateIdle>());
        return;
    }
}

void EnemyStateAttack::Exit(Enemy* enemy) {
    // Execute once when exiting the attack state
    // Example: Disable the weapon collider, reset the hit flag, and perform other cleanup operations.
}