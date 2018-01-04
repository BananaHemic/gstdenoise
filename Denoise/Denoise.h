#pragma once

#include <gst/gst.h>
#include <gst/audio/audio.h>
#include <gst/audio/gstaudiofilter.h>
#include <string.h>

#include <fftw3.h>
extern "C" {
#include "extra_functions.h"
#include "masking.h"
#include "estimate_noise_spectrum.h"
#include "spectral_processing.h"
}

//STFT default values
#define FFT_SIZE 2048 //Size of the fft transform and frame
#define INPUT_WINDOW 3 //0 HANN 1 HAMMING 2 BLACKMAN 3 VORBIS Input windows for STFT algorithm
#define OUTPUT_WINDOW 3 //0 HANN 1 HAMMING 2 BLACKMAN 3 VORBIS Output windows for STFT algorithm
#define OVERLAP_FACTOR 4 //4 is 75% overlap Values bigger than 4 will rescale correctly (if Vorbis windows is not used)

#define NOISE_ARRAY_STATE_MAX_SIZE 8192 //max alloc size of the noise_thresholds to save with the session. This will consider upto fs of 192 kHz with aprox 23hz resolution

GST_DEBUG_CATEGORY_STATIC(denoise_debug);
#define GST_CAT_DEFAULT denoise_debug

typedef struct _GstDenoise GstDenoise;
typedef struct _GstDenoiseClass GstDenoiseClass;

/* These are boilerplate cast macros and type check macros */
#define GST_TYPE_DENOISE \
  (gst_denoise_get_type())
#define GST_DENOISE(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_DENOISE,GstDenoise))
#define GST_DENOISE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_DENOISE,GstDenoiseClass))
#define GST_IS_DENOISE(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_DENOISE))
#define GST_IS_DENOISE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_DENOISE))

struct _GstDenoise
{
	GstAudioFilter audiofilter;

	/* here you can add additional per-instance
	* data such as properties */
	const float* input; //input of samples from host (changing size)
	float* output; //output of samples to host (changing size)
	float samp_rate; //Sample rate received from the host

	//Parameters for the algorithm (user input)
	float amount_of_reduction; //Amount of noise to reduce in dB
	float noise_thresholds_offset; //This is to scale the noise profile (over subtraction factor) in dB
	float release; //Release time
	float masking; //Masking scaling
	float whitening_factor_pc;	//Whitening amount of the reduction percentage
	bool noise_learn_state; //Learn Noise state (Manual-Off-Auto)
	float adaptive_state; //Autocapture switch
	float reset_profile; //Reset Noise switch
	bool residual_listen; //For noise only listening
	float transient_protection; //Multiplier for thresholding onsets with rolling mean
	float enable; //For soft bypass (click free bypass)
	float* report_latency; //Latency necessary
	char* file_location;

	//Parameters values and arrays for the STFT
	int fft_size; //FFTW input size
	int fft_size_2; //FFTW half input size
	int window_option_input; //Type of input Window for the STFT
	int window_option_output; //Type of output Window for the STFT
	float overlap_factor; //oversampling factor for overlap calculations
	float overlap_scale_factor; //Scaling factor for conserving the final amplitude
	int hop; //Hop size for the STFT
	float* input_window; //Input Window values
	float* output_window; //Output Window values

	//Algorithm exta variables
	float tau; //time constant for soft bypass
	float wet_dry_target; //softbypass target for softbypass
	float wet_dry; //softbypass coeff
	float reduction_coeff; //Gain to apply to the residual noise
	float release_coeff; //Release coefficient for Envelopes
	float amount_of_reduction_linear;	//Reduction amount linear value
	float thresholds_offset_linear;	//Threshold offset linear value
	float whitening_factor; //Whitening amount of the reduction

	//Buffers for processing and outputting
	int input_latency;
	float* in_fifo; //internal input buffer
	float* out_fifo; //internal output buffer
	float* output_accum; //FFT output accumulator
	int read_ptr; //buffers read pointer

	//FFTW related arrays
	float* input_fft_buffer;
	float* output_fft_buffer;
	fftwf_plan forward;
	fftwf_plan backward;

	//Arrays and variables for getting bins info
	float* fft_p2; //power spectrum
	float* fft_magnitude; //magnitude spectrum
	float* fft_phase; //phase spectrum

	//noise related
	float* noise_thresholds_p2; //captured noise profile power spectrum
	float* noise_thresholds_scaled; //captured noise profile power spectrum scaled by oversubtraction
	bool noise_thresholds_availables; //indicate whether a noise profile is available or no
	float noise_window_count; //Count windows for mean computing

	//smoothing related
	float* smoothed_spectrum; //power spectrum to be smoothed
	float* smoothed_spectrum_prev; //previous frame smoothed power spectrum for envelopes

	//Transient preservation related
	float* transient_preserv_prev; //previous frame smoothed power spectrum for envelopes
	float tp_r_mean;
	bool transient_present;
	float tp_window_count;

	//Reduction gains
	float* Gk; //definitive gain

	//Ensemble related
	float* residual_spectrum;
	float* denoised_spectrum;
	float* final_spectrum;

	//whitening related
	float* residual_max_spectrum;
	float max_decay_rate;
	float whitening_window_count;

	//Loizou algorithm
	float* auto_thresholds; //Reference threshold for louizou algorithm
	float* prev_noise_thresholds;
	float* s_pow_spec;
	float* prev_s_pow_spec;
	float* p_min;
	float* prev_p_min;
	float* speech_p_p;
	float* prev_speech_p_p;

	//masking
	float* bark_z;
	float* absolute_thresholds; //absolute threshold of hearing
	float* SSF;
	float* spl_reference_values;
	float* unity_gain_bark_spectrum;
	float* spreaded_unity_gain_bark_spectrum;
	float* alpha_masking;
	float* beta_masking;
	float* input_fft_buffer_at;
	float* output_fft_buffer_at;
	fftwf_plan forward_at;
};

struct _GstDenoiseClass
{
	GstAudioFilterClass audiofilter_class;
};

G_DEFINE_TYPE(GstDenoise, gst_denoise,
	GST_TYPE_AUDIO_FILTER);

static void gst_denoise_set_property(GObject * object,
	guint prop_id, const GValue * value, GParamSpec * pspec);
static void gst_denoise_get_property(GObject * object,
	guint prop_id, GValue * value, GParamSpec * pspec);

static gboolean gst_denoise_setup(GstAudioFilter * filter,
	const GstAudioInfo * info);
static GstFlowReturn gst_denoise_filter(GstBaseTransform * bt,
	GstBuffer * outbuf, GstBuffer * inbuf);
static GstFlowReturn
gst_denoise_filter_inplace(GstBaseTransform * base_transform,
	GstBuffer * buf);
static gboolean
gst_denoise_stop(GstBaseTransform *trans);
