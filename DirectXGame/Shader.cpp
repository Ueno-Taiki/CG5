#include "Shader.h"
#include "MiscUtility.h"
#include <d3dcompiler.h>  // D3DCompileFromFile
#include <cassert>  // assert
#include <dxcapi.h>
#pragma comment(lib, "dxcompiler.lib")

// シェーダーファイルを読み込み、コンパイルする
void Shader::Load(const std::wstring& filePath, const std::wstring& shaderModel) {
	ID3D10Blob* shaderBlob = nullptr;
	ID3D10Blob* errorBlob = nullptr;

	// wstring => string 文字列変換
	std::string mbshaderModel = ConverString(shaderModel);

	// Shaderのコンパイル
	HRESULT hr = D3DCompileFromFile(
		filePath.c_str(), // シェーダーファイル名
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,  // インクルード可能にする
		"main", mbshaderModel.c_str(),  // エントリーポイント名、シェーダーモデル指定
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, // デバック用設定
		0, &shaderBlob, &errorBlob);

	//　エラーが発生した場合、止める
	if (FAILED(hr)) {
		if (errorBlob) {
			OutputDebugStringA(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
			errorBlob->Release();
		}
		assert(false);
	}
	// 生成したshaderBlobをとっておく
	blob_ = shaderBlob;
}

// シェーダーファイルを読み込み、コンパイルする
void Shader::LoadDxc(const std::wstring& filePath, const std::wstring& shaderModel) { 
	// DXCを初期化
	static IDxcUtils* dxcUtils = nullptr;
	static IDxcCompiler3* dxcCompiler = nullptr;
	static IDxcIncludeHandler* includeHandler = nullptr;

	HRESULT hr;
	if (dxcUtils == nullptr) {
		hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils));
		assert(SUCCEEDED(hr)); // うまくいかなかったときは起動できない
	}
	if (dxcCompiler == nullptr) {
		hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler));
		assert(SUCCEEDED(hr)); // うまくいかなかったときは起動
	}
	if (includeHandler == nullptr) {
		hr = dxcUtils->CreateDefaultIncludeHandler(&includeHandler);
		assert(SUCCEEDED(hr)); // うまくいかなかったときは起動
	}

	// 1.hlslファイルを読む
	IDxcBlobEncoding* shaderSource = nullptr;
	hr = dxcUtils->LoadFile(filePath.c_str(), nullptr, &shaderSource);
	assert(SUCCEEDED(hr));

	// 読み込んだファイルの内容をDxcBufferに設定する
	DxcBuffer shaderSourceBuffer{};
	shaderSourceBuffer.Ptr = shaderSource->GetBufferPointer();
	shaderSourceBuffer.Size = shaderSource->GetBufferSize();
	shaderSourceBuffer.Encoding = DXC_CP_UTF8;

	// 2.Compileする
	LPCWSTR arguments[] = {
		filePath.c_str(), // コンパイル対象のhlslファイル名
		L"-E",
		L"main", // エントリーポイント名の指定
		L"-T",
		shaderModel.c_str(), // ShaderProfileの設定
		L"-Zi",
		L"-Qembed_debug", // デバック用の情報を埋め込む
		L"-Od", // 最適化を外しておく
		L"-Zpr", // メモリレイアウトは行優先
	};

	// 実際にShaderをコンパイルする
	IDxcResult* shaderResult = nullptr;
	hr = dxcCompiler->Compile(
		&shaderSourceBuffer, // 読み込んだファイル
		arguments, // コンパイルオプション
		_countof(arguments), // コンパイルオプションの数
		includeHandler, // includeが含まれた諸々
		IID_PPV_ARGS(&shaderResult) // コンパイル結果
	);
	// コンパイルエラーではなくdxcが起動できないなど致命的な状況
	assert(SUCCEEDED(hr));

	// 3.警告・エラーがでていないか確認する
	IDxcBlobUtf8* shaderError = nullptr;
	IDxcBlobWide* nameBlob = nullptr;
	shaderResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&shaderError), &nameBlob);
	if (shaderError != nullptr && shaderError->GetStringLength() != 0) {
		OutputDebugStringA(shaderError->GetStringPointer());
		assert(false);
	}

	// 4.Compile結果を受け取る
	IDxcBlob* shaderBlob = nullptr;
	hr = shaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), &nameBlob);
	assert(SUCCEEDED(hr));

	// もう使わないリソースを解放
	shaderSource->Release();
	shaderResult->Release();

	// 実行用のバイナリを取っておく
	dxcBlob_ = shaderBlob;
}

// コンパイル済みのシェーダーファイルを返す
ID3DBlob* Shader::GetBlob() {
	return blob_;
}

IDxcBlob* Shader::GetDxcBlob() {
	return dxcBlob_;
}

// コンストラクタ
Shader::Shader() {}

//デストラクタ
Shader::~Shader() { 
	if (blob_ != nullptr) {
		blob_->Release();
		blob_ = nullptr;
	}
	if (dxcBlob_ != nullptr) {
		dxcBlob_->Release();
		dxcBlob_ = nullptr;
	}
}
