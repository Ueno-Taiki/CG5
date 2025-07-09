#pragma once

#include "KamataEngine.h" // WorldTransformクラスの利用

class WorldTransformEx :
    public KamataEngine::WorldTransform {

public:
    // Affin変換行列の生成と定数バッファの転送を行う
    void UpdateMatrix();

    // Affine変換行列の生成
    KamataEngine::Matrix4x4 MakeAffineMatrix();
};

