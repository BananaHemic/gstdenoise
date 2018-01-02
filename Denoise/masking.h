#pragma once

//masking thresholds values recomended by virag
#define ALPHA_MAX 6.f
#define ALPHA_MIN 1.f
#define BETA_MAX 0.02f
#define BETA_MIN 0.0f

//extra values
#define N_BARK_BANDS 25
#define AT_SINE_WAVE_FREQ 1000.f
#define REFERENCE_LEVEL 90.f //dbSPL level of reproduction

#define BIAS 0
#define HIGH_FREQ_BIAS 20.f
#define S_AMP 1.f

#define ARRAYACCESS(a, i, j) ((a)[(i) * N_BARK_BANDS + (j)]) //This is for SSF Matrix recall

//Proposed by Sinha and Tewfik and explained by Virag
const float relative_thresholds[N_BARK_BANDS] = { -16.f, -17.f, -18.f, -19.f, -20.f, -21.f, -22.f, -23.f, -24.f, -25.f, -25.f, -25.f, -25.f, -25.f, -25.f, -24.f, -23.f, -22.f, -19.f, -18.f, -18.f, -18.f, -18.f, -18.f, -18.f};


void compute_bark_mapping(float * bark_z, int fft_size_2, int srate);

void compute_SSF(float * SSF);

void convolve_with_SSF(float * SSF, float * bark_spectrum, float * spreaded_spectrum);

void compute_bark_spectrum(float * bark_z, float * bark_spectrum, float * spectrum, float * intermediate_band_bins, float * n_bins_per_band);

void spl_reference(float * spl_reference_values, int fft_size_2, int srate, float * input_fft_buffer_at, float * output_fft_buffer_at, fftwf_plan * forward_at);

void convert_to_dbspl(float * spl_reference_values, float * masking_thresholds, int fft_size_2);

void compute_absolute_thresholds(float * absolute_thresholds, int fft_size_2, int srate);

float compute_tonality_factor(float * spectrum, float * intermediate_band_bins, float * n_bins_per_band, int band);

void compute_masking_thresholds(float * bark_z, float * absolute_thresholds, float * SSF, float * spectrum, int fft_size_2, float * masking_thresholds, float * spreaded_unity_gain_bark_spectrum, float * spl_reference_values);

void compute_alpha_and_beta(float * fft_p2, float * noise_thresholds_p2, int fft_size_2, float * alpha_masking, float * beta_masking, float * bark_z, float * absolute_thresholds, float * SSF, float * spreaded_unity_gain_bark_spectrum, float * spl_reference_values, float masking_value, float reduction_value);
