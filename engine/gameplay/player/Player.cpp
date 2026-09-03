#include "Player.h"
#include <algorithm>
#include "ImGuiManager.h"
#include "Object3dCommon.h"
#include "ModelManager.h"
#include "DirectXCommon.h"
#include "Object3d.h"
#include "PlayerStateIdle.h"
#include "PlayerStateMove.h"

Player::~Player() {
}

void Player::Initialize() {

	ModelManager::GetInstance()->Initialize(DirectXCommon::GetInstance());

	ModelManager::GetInstance()->LoadModel("resources/player", "body.obj");
	ModelManager::GetInstance()->LoadModel("resources/player", "head.obj");
	ModelManager::GetInstance()->LoadModel("resources/player", "arm.obj");
	ModelManager::GetInstance()->LoadModel("resources/player", "leg.obj");
	ModelManager::GetInstance()->LoadModel("resources/player", "playerWeapon.obj");

	for (int i = 0; i < kPlayerPartCount; ++i) {
		playerTransforms_[i] = std::make_unique<WorldTransform>();
		playerTransforms_[i]->Initialize();

		playerParts_[i] = std::make_unique<Object3d>();
		playerParts_[i]->Initialize(Object3dCommon::GetInstance(), playerTransforms_[i].get());
	}

	for (int i = 0; i < kPlayerPartCount; ++i) {
		if (i == BODY) continue;
		playerTransforms_[i]->parent_ = playerTransforms_[BODY].get(); 
	}

	playerParts_[ARM_R]->SetModel("arm.obj");
	playerParts_[ARM_L]->SetModel("arm.obj");
	playerParts_[LEG_R]->SetModel("leg.obj");
	playerParts_[LEG_L]->SetModel("leg.obj");
	playerParts_[BODY]->SetModel("body.obj");
	playerParts_[HEAD]->SetModel("head.obj");
	playerParts_[WEAPON]->SetModel("playerWeapon.obj");

	
	playerTransforms_[ARM_R]->translate_ = { 4.7f, 3.2f, 0.0f };
	playerTransforms_[ARM_L]->translate_ = { -4.7f, 3.2f, 0.0f };
	playerTransforms_[LEG_R]->translate_ = { -2.4f, -2.1f, 0.1f };
	playerTransforms_[LEG_L]->translate_ = { 2.5f, -2.1f, 0.1f };
	playerTransforms_[BODY]->translate_ = { 0.0f, 5.0f, 0.0f };
	playerTransforms_[HEAD]->translate_ = { 0.2f, 3.9f, 0.0f };
	playerTransforms_[WEAPON]->translate_ = { 4.7f, 3.2f, 0.0f };

	playerTransforms_[BODY]->scale_ = { 0.5f, 0.5f, 1.0f };
	playerTransforms_[HEAD]->scale_ = { 2.5f, 1.3f, 1.0f };
	playerTransforms_[WEAPON]->scale_ = { 1.0f, 4.5f, 1.0f };
	if (camera_) {
		
	}


	originalWeaponAngleX_ = playerTransforms_[WEAPON]->rotate_.x;

	ChangeState(std::make_unique<PlayerStateIdle>());
}

void Player::Updata() {


	if (currentState_) {
		currentState_->Update(this);
	}

	
	for (int i = 0; i < kPlayerPartCount; ++i) {
		if (playerTransforms_[i]) {
			playerTransforms_[i]->UpdateMatrix();
			playerTransforms_[i]->TransferMatrix();
		}
	}

	


	Vector3& pos = playerTransforms_[BODY]->translate_;
	float fieldLimitX = 150.0f;
	float fieldLimitZ = 150.0f;
	pos.x = std::clamp(pos.x, -fieldLimitX, fieldLimitX);
	pos.z = std::clamp(pos.z, -fieldLimitZ, fieldLimitZ);

	position = pos;

	playerTransforms_[BODY]->UpdateMatrix();
	playerTransforms_[BODY]->TransferMatrix();

	for (int i = 0; i < kPlayerPartCount; ++i) {
		if (i == BODY) continue;
		playerTransforms_[i]->UpdateMatrix();
		playerTransforms_[i]->TransferMatrix();
	}


	if (!isAttacking_ && Input::GetInstance()->TriggerKey(DIK_SPACE)) {
		isAttacking_ = true;
		attackTimer_ = 0;
		originalWeaponAngleX_ = playerTransforms_[WEAPON]->rotate_.x;
	}

	if (isAttacking_) {
		const float maxSwing = 3.0f; 
		float t = static_cast<float>(attackTimer_) / static_cast<float>(attackDuration_);


		float swingAngle = 0.0f;
		if (t <= 0.5f) {
			swingAngle = maxSwing * (t * 2.0f); // 0 → max
		} else {
			swingAngle = maxSwing * (2.0f - t * 2.0f); // max → 0
		}

		playerTransforms_[WEAPON]->rotate_.x = originalWeaponAngleX_ + swingAngle;

		attackTimer_++;

		if (!hasHit_ && attackTimer_ == 15) {
			 
			hasHit_ = HitCheck();
		}

		if (attackTimer_ >= attackDuration_) {
			isAttacking_ = false;
			hasHit_ = false; 
			attackTimer_ = 0;
			playerTransforms_[WEAPON]->rotate_.x = originalWeaponAngleX_; 
		}
	}

#ifdef _DEBUG
	ImGui::Begin("Player Model Transform");

	static const char* partNames[] = {
		"ARM_R", "ARM_L", "LEG_R", "LEG_L", "BODY", "HEAD"," WEAPON"
	};

	for (int i = 0; i < kPlayerPartCount; ++i) {
		if (ImGui::CollapsingHeader(partNames[i])) {
			ImGui::DragFloat3("Position", &playerTransforms_[i]->translate_.x, 0.1f);
			ImGui::DragFloat3("Rotation", &playerTransforms_[i]->rotate_.x, 0.1f);
			ImGui::DragFloat3("Scale", &playerTransforms_[i]->scale_.x, 0.1f);
		}
	}

	ImGui::End();
#endif

}

void Player::Draw() {

	for (auto& part : playerParts_) {
		part->Draw();
	}
	
}

void Player::ChangeState(std::unique_ptr<IPlayerState> newState) {
	if (currentState_) {
		currentState_->Exit(this);
	}
	currentState_ = std::move(newState);
	if (currentState_) {
		currentState_->Enter(this);
	}
}

void Player::HitEffectDraw() {
	if (hasHit_) {
		effectLibrary_->DrawPrimitive();
	}
}

void Player::SetCamera(Camera* camera) {
	camera_ = camera;
	for (auto& part : playerParts_) {
		if (part) {
			part->SetCamera(camera_);
		}
	}
}

bool Player::HitCheck() {
	
	Vector3 weaponModelSize = { 1.0f, 4.5f, 1.0f };

	OBB weaponOBB = MyMath::MakeOBB(*playerTransforms_[WEAPON], weaponModelSize);

	bool didHit = false;

	
	for (auto& enemy : enemies_) {
		
		Vector3 enemyModelSize = { 12.0f, 3.0f, 3.0f };

		OBB enemyOBB = MyMath::MakeOBB(enemy->GetWorldTransform(), enemyModelSize);

	
		if (MyMath::IsOBBCollision(weaponOBB, enemyOBB)) {
			enemy->OnHit(10.0f);

			
			Vector3 hitPos = enemyOBB.center;

			effectLibrary_->EmitPrimitive(hitPos, *randomEngine_);
			didHit = true;
		}
	}

	return didHit;
}

const Vector3& Player::GetWeaponPosition() {
	return playerTransforms_[WEAPON]->translate_;
}
void Player::SetEffectLibrary(ParticleEffectLibrary* effectLibrary) {
	effectLibrary_ = effectLibrary;
}

void Player::SetRandomEngine(std::mt19937* engine) {
	randomEngine_ = engine;
}

Vector3 Player::GetWeaponWorldPosition() {
	return MyMath::GetTranslate(playerTransforms_[WEAPON]->matWorld_);
}




