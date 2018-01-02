#pragma once

#include <math.h>
#include <float.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>

#define MAX(a,b) (((a) > (b)) ? (a) : (b))
#define MIN(a,b) (((a) < (b)) ? (a) : (b))

//Window types
#define HANN_WINDOW 0
#define HAMMING_WINDOW 1
#define BLACKMAN_WINDOW 2
#define VORBIS_WINDOW 3

#define M_PI 3.14159265358979323846f

#define SP_MAX_NUM 100 //Max number of spectral peaks to find
#define SP_THRESH 0.1f //Threshold to discriminate peaks (high value to discard noise) Linear 0<>1
#define SP_USE_P_INTER true //Use parabolic interpolation
#define SP_MAX_FREQ 16000.f //Highest frequency to search for peaks
#define SP_MIN_FREQ 40.f //Lowest frequency to search for peaks

#define SE_RESOLUTION 100.f //Spectral envelope resolution

#define WHITENING_DECAY_RATE 1000.f //Deacay in ms for max spectrum for whitening
#define WHITENING_FLOOR 0.02f //Minumum max value posible

#define TP_UPPER_LIMIT 5.f //This correspond to the upper limit of the adaptive threshold multiplier. Should be the same as the ttl configured one

/**
* FFT peak struct. To describe spectral peaks.
*/
typedef struct
{
	float magnitude;
	int position;
} FFTPeak;

float sanitize_denormal(float value);

int sign(float x);

int next_pow_two(int x);

int nearest_odd(int x);

int nearest_even(int x);

float from_dB(float gdb);

float to_dB(float g);

float bin_to_freq(int i, float samp_rate, int N);

int freq_to_bin(float freq, float samp_rate, int N);

void parabolic_interpolation(float left_val, float middle_val, float right_val, int current_bin, float * result_val, int * result_bin);

void initialize_array(float * array, float value, int size);

bool is_empty(float * spectrum, int N);

float max_spectral_value(float * spectrum, int N);

float min_spectral_value(float * spectrum, int N);

float spectral_mean(float * a, int m);

float spectral_addition(float * a, int m);

float spectral_median(float * x, int n);

float spectral_moda(float * x, int n);

void get_normalized_spectum(float * spectrum, int N);

float spectral_flux(float * spectrum, float * spectrum_prev, float N);

float high_frequency_content(float * spectrum, float N);

void spectral_envelope(int fft_size_2, float * fft_p2, int samp_rate, float * spectral_envelope_values);

void spectral_peaks(int fft_size_2, float * fft_p2, FFTPeak * spectral_peaks, int * peak_pos, int * peaks_count, int samp_rate);

float spectrum_p_norm(float * spectrum, float N, float p);

void spectral_whitening(float * spectrum, float b, int N, float * max_spectrum, float * whitening_window_count, float max_decay_rate);

void spectrum_adaptive_time_smoothing(int fft_size_2, float * spectrum_prev, float * spectrum, float * noise_thresholds, float * prev_beta, float coeff);

void apply_time_envelope(float * spectrum, float * spectrum_prev, float N, float release_coeff);

bool transient_detection(float * fft_p2, float * transient_preserv_prev, float fft_size_2, float * tp_window_count, float * tp_r_mean, float transient_protection);

float blackman(int k, int N);

float hanning(int k, int N);

float hamming(int k, int N);

float vorbis(int k, int N);

void fft_window(float * window, int N, int window_type);

float get_window_scale_factor(float * input_window, float * output_window, int frame_size);

void fft_pre_and_post_window(float * input_window, float * output_window, int frame_size, int window_option_input, int window_option_output, float * overlap_scale_factor);

void get_info_from_bins(float * fft_p2, float * fft_magnitude, float * fft_phase, int fft_size_2, int fft_size, float * fft_buffer);
