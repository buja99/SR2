#pragma once

struct GrayscaleSettings {
    float strength;
    float pad[3]; // 16
};
struct VignetteSettings {
	float vignetteStrength;
    float pad[3];
};

struct RadialBlurSettings {
    float centerX;       // float 4
    float centerY;       // float 4
    float blurStrength;  // float 4
    int   numSamples;    // int   4
    float pad[3];
};

enum class PostEffectMode {
    None,
    Grayscale,
    Vignette,
    RadialBlur
};