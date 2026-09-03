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

	dxCommon_->RenderTexturePreDraw();
	srvManager_->PreDraw();
	sceneManager_->Draw();
	dxCommon_->RenderTexturePostDraw();

	if (!PostProcessManager::GetInstance()->HasAnyEffects()) {
		// If no effects are registered in the chain, copy the original image directly to the swap chain.
		dxCommon_->CopyRenderTextureToSwapChain();
	} else {
		// If at least one effect is registered in the chain, delegate post-processing to the manager.
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