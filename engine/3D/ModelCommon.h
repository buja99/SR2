#pragma once
#include <memory>

class DirectXCommon;

class ModelCommon
{

public:

	void Initialize(DirectXCommon* dxCommon);

	void Finalize();

	DirectXCommon* GetDxCommon() const { return dxCommon_; }



private:
		
	DirectXCommon* dxCommon_ = nullptr;

};

