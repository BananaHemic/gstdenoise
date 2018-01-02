#pragma once

void compute_auto_thresholds(float * auto_thresholds, float fft_size, float fft_size_2, float samp_rate);

static void estimate_noise_loizou(float * thresh, int fft_size_2, float * p2, float * s_pow_spec, float * prev_s_pow_spec, float * noise_thresholds_p2, float * prev_noise, float * p_min, float * prev_p_min, float * speech_p_p, float * prev_speech_p_p);

void adapt_noise(float * p2, int fft_size_2, float * noise_thresholds_p2, float * thresh, float * prev_noise_thresholds, float * s_pow_spec, float * prev_s_pow_spec, float * p_min, float * prev_p_min, float * speech_p_p, float * prev_speech_p_p);

void get_noise_statistics(float * fft_p2, int fft_size_2, float * noise_thresholds_p2, float window_count);
