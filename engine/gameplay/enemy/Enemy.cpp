#include "Enemy.h"
#include "ImGuiManager.h"
#include "Object3dCommon.h"
#include "ModelManager.h"
#include "DirectXCommon.h"
#include "Object3d.h"
#include "EnemyStateIdle.h"
Enemy::Enemy() {
}
void Enemy::Initialize() {
    ModelManager::GetInstance()->Initialize(DirectXCommon::GetInstance());
    ModelManager::GetInstance()->LoadModel("resources/enemy", "enemyTest.obj");

    enemyWorldTransform_ = std::make_unique<WorldTransform>();
	enemyWorldTransform_->Initialize();

	enemyModel_ = std::make_unique<Object3d>();
	enemyModel_->Initialize(Object3dCommon::GetInstance(), enemyWorldTransform_.get());
    enemyModel_->SetModel("enemyTest.obj");
    enemyModel_->SetUseEnvironmentMap(true);
    enemyModel_->SetEnvironmentMap("resources/rostock_laage_airport_4k.dds");
	enemyWorldTransform_->translate_ = { 0.0f,5.0f,0.0f };
	//enemyWorldTransform_->scale_ = { 12.0f, 3.0f, 3.0f };
    isHit_ = false;
	ChangeState(std::make_unique<EnemyStateIdle>());
}

void Enemy::Update() {


    if (currentState_) {
        currentState_->Update(this);
    }

	enemyWorldTransform_->UpdateMatrix();
    enemyWorldTransform_->TransferMatrix();

}

void Enemy::Draw() {
	enemyModel_->Draw();
   
}

int Enemy::GetType() const {
    return 0; 
}

void Enemy::OnHit(float damage) {
    
    OutputDebugStringA("Enemy::OnHit hit!\n");

    hp_ -= damage;
    if (hp_ <= 0.0f) {
        isDead_ = true;
    }

    Vector3 pos = enemyWorldTransform_->translate_;
    //pos.y += 2.0f;
   
     // effectLibrary_->EmitHitEffect(pos, ...);
     isHit_ = true;
}
void Enemy::SetCamera(Camera* camera) {
	camera_ = camera;
	
}
WorldTransform& Enemy::GetWorldTransform() {
	return *enemyWorldTransform_;
}

void Enemy::ChangeState(std::unique_ptr<IEnemyState> newState) {
    if (currentState_) {
        currentState_->Exit(this); // Exit the current state
    }
    currentState_ = std::move(newState); // Switch to the new state
    if (currentState_) {
        currentState_->Enter(this); // Enter the new state
    }
}


