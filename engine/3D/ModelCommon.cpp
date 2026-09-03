#include "ModelCommon.h"
#include "DirectXCommon.h"

void ModelCommon::Initialize(DirectXCommon* dxCommon)
{

	dxCommon_ = dxCommon;

}

void ModelCommon::Finalize() {
	//dxCommon_ = nullptr;
}
