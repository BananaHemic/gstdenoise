#pragma once

void preprocessing(float noise_thresholds_offset, float * fft_p2, float * noise_thresholds_p2, float * noise_thresholds_scaled, float * smoothed_spectrum, float * smoothed_spectrum_prev, int fft_size_2, float * bark_z, float * absolute_thresholds, float * SSF, float release_coeff, float * spreaded_unity_gain_bark_spectrum, float * spl_reference_values, float * alpha_masking, float * beta_masking, float masking_value, bool adaptive_state, float reduction_value, float * transient_preserv_prev, float * tp_window_count, float * tp_r_mean, bool * transient_present, float transient_protection);

void spectral_gain(float * fft_p2, float * noise_thresholds_p2, float * noise_thresholds_scaled, float * smoothed_spectrum, int fft_size_2, bool adaptive, float * Gk, float transient_protection, bool transient_present);

void denoised_calulation(int fft_size, float * output_fft_buffer, float * denoised_spectrum, float * Gk);

void residual_calulation(int fft_size, float * output_fft_buffer, float * residual_spectrum, float * denoised_spectrum, float whitening_factor, float * residual_max_spectrum, float * whitening_window_count, float max_decay_rate);

void final_spectrum_ensemble(int fft_size, float * final_spectrum, float * residual_spectrum, float * denoised_spectrum, float reduction_amount, bool noise_listen);

void soft_bypass(float * final_spectrum, float * output_fft_buffer, float wet_dry, int fft_size);
