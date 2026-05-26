#include "MyGame.h"
#include "PostProcessManager.h"



void MyGame::Initialize() {

	Framework::Initialize();

	sceneFactory_ = std::make_unique<SceneFactory>();
	sceneManager_->SetSceneFactory(std::move(sceneFactory_));

	//sceneManager_->ChangeScene("GAME");
	sceneManager_->ChangeScene("TITLE");

}

void MyGame::Finalize() {

	Framework::Finalize();

}

void MyGame::Update() {

	Framework::Update();

}

void MyGame::Draw() {

	// 1. Off-Screen Rendering
	dxCommon_->RenderTexturePreDraw();
	srvManager_->PreDraw();
	sceneManager_->Draw();
	dxCommon_->RenderTexturePostDraw();
	//dxCommon_->PreDraw();
	// 2. Copy to Swap Chain
	//dxCommon_->CopyRenderTextureToSwapChain();
	PostProcessManager::GetInstance()->Initialize(dxCommon_->GetDevice().Get());

	if (!PostProcessManager::GetInstance()->HasAnyEffects()) {
		// 체인에 등록된 이펙트가 하나도 없으면 원본 화면을 그대로 스왑체인(화면)에 복사
		dxCommon_->CopyRenderTextureToSwapChain();
	} else {
		// 체인에 이펙트가 하나라도 있으면 매니저에게 통합 렌더링(후처리) 위임
		PostProcessManager::GetInstance()->Draw(
			dxCommon_->GetCommandList().Get(),
			dxCommon_->GetOffscreenSRVIndex()
		);
	}
#ifdef _DEBUG
	imGuiManager_->Draw();
#endif // _DEBUG

	// 3. Final Rendering Complete
	dxCommon_->PostDraw();


}