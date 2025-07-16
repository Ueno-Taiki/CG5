#include "Test.hlsli"

Texture2D<float32_t4> gTexture : register(t0);  // SRV
SamplerState gSampler : register(s0); // Sampler

struct PixelShaderOutput {
   float32_t4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input) {
    PixelShaderOutput output;

    output.color = gTexture.Sample(gSampler, input.texcoord);

    float32_t2 uv = input.texcoord;
    float32_t4 textureColor = gTexture.Sample(gSampler, uv);
    
    // grayscale
    float32_t value = dot(textureColor.rgb, float32_t3(0.2125f, 0.7154f, 0.0721f));
    output.color = float32_t4(value, value, value, textureColor.a);
    
    // 周囲を0に、中心になるほど明るくなるように計算で調節
    float32_t2 correct = input.texcoord * (1.0f - input.texcoord.yx);
    // correctだけで計算すると中心の最大級が0.0625で暗すぎるのでScaleで調節。
    float vignette = correct.x * correct.y * 16.0f;
    // とりあいず0.8乗
    vignette = saturate(pow(vignette, 0.8f));
    // 係数として乗算
    output.color.rgb *= vignette;

    return output;
}