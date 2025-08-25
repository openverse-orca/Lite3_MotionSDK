#ifndef SQUARE_WAVE_H
#define SQUARE_WAVE_H

#include <cmath>
#include <iostream>

class SquareWaveGenerator {
private:
    float _square_wave_phase = 0.0f;
    float dt = 0.0f;
    
    struct FootSquareWaveConfig {
        float p5;
        float phase_freq;
        float eps;
    };
    
    FootSquareWaveConfig* _foot_square_wave = nullptr;

public:
    // 设置时间步长
    void set_dt(float time_step);

    // 设置方波配置
    void set_foot_square_wave(float p5, float phase_freq, float eps);

    // 平滑方波函数
    static float smooth_sqr_wave_np(float phase, float phase_freq, float eps);

    // 计算方波信号
    float compute_square_wave(float command_lin_vel_x);

    // 析构函数
    ~SquareWaveGenerator();
};

#endif // SQUARE_WAVE_H