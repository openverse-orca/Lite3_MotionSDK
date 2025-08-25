#include "square_wave.h"

// 设置时间步长
void SquareWaveGenerator::set_dt(float time_step) {
    dt = time_step;
}

// 设置方波配置
void SquareWaveGenerator::set_foot_square_wave(float p5, float phase_freq, float eps) {
    if (_foot_square_wave == nullptr) {
        _foot_square_wave = new FootSquareWaveConfig();
    }
    _foot_square_wave->p5 = p5;
    _foot_square_wave->phase_freq = phase_freq;
    _foot_square_wave->eps = eps;
}

// 平滑方波函数
float SquareWaveGenerator::smooth_sqr_wave_np(float phase, float phase_freq, float eps) {
    float p = 2.0f * M_PI * phase * phase_freq;
    float numerator = std::sin(p);
    float sin_p = std::sin(p);
    float denominator = 2.0f * std::sqrt(sin_p * sin_p + eps * eps);
    return numerator / denominator + 0.5f;
}

// 计算方波信号
float SquareWaveGenerator::compute_square_wave(float command_lin_vel_x) {
    if (_foot_square_wave == nullptr) {
        return 0.0f;
    }
    
    // 更新相位
    _square_wave_phase = std::fmod(_square_wave_phase + dt, 1.0f);
    
    if (command_lin_vel_x == 0.0f) {
        return _foot_square_wave->p5;
    } else {
        return smooth_sqr_wave_np(_square_wave_phase, 
                                 _foot_square_wave->phase_freq, 
                                 _foot_square_wave->eps);
    }
}

// 析构函数
SquareWaveGenerator::~SquareWaveGenerator() {
    delete _foot_square_wave;
} 