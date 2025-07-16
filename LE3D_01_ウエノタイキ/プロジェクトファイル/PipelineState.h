#pragma once

#include <d3dx12.h> // ID3D12RootSignature

class PipelineState {
public:
	// 初期化
	void Create(D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc);
	// ゲッター
	ID3D12PipelineState* Get();

	// コンストラ
	PipelineState();
	// デストラクタ
	~PipelineState();

private:
	// パイプラインステート
	ID3D12PipelineState* pipelineState_ = nullptr;
};

