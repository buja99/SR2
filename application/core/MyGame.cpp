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
	PostEffectMode currentMode = PostProcessManager::GetInstance()->GetPostEffectMode();

	if (currentMode == PostEffectMode::None) {
		// When the effect is disabled, draw the original screen as-is
		dxCommon_->CopyRenderTextureToSwapChain();
	} else {
		// When the effect is enabled, delegate integrated rendering to the newly created manager
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