/* GStreamer audio noise filter
* Copyright (C) <2018> Bananahemic <...>
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
* DEALINGS IN THE SOFTWARE.
*
* Alternatively, the contents of this file may be used under the
* GNU Lesser General Public License Version 2.1 (the "LGPL"), in
* which case the following provisions apply instead of the ones
* mentioned above:
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Library General Public
* License as published by the Free Software Foundation; either
* version 2 of the License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Library General Public License for more details.
*
* You should have received a copy of the GNU Library General Public
* License along with this library; if not, write to the
* Free Software Foundation, Inc., 59 Temple Place - Suite 330,
* Boston, MA 02111-1307, USA.
*/

/**
* SECTION: Denoise
*
* Allows removing noise from live and non-live audio streams
* Either through automatically guessing the noise, or by
* Building a noise model by providing the element just noise
* and then later applying the noise
*
* <refsect2>
* <title>Example launch line</title>
* |[
* gst-launch -v -m autoaudiosrc ! audioconvert ! denoise ! audioconvert ! autoaudiosink
* ]|
* </refsect2>
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "Denoise.h"

enum
{
	/* FILL ME */
	LAST_SIGNAL
};

enum
{
	PROP_0,
	PROP_RELEASE_TIME,
	PROP_INTENSITY,
	PROP_NOISE_OFFSET,
	PROP_WHITENING_FACTOR,
	PROP_MASKING,
	PROP_TRANSIENT_PROTECTION,
	PROP_RESIDUAL,
	PROP_AUTO_LEARN,
	PROP_BUILD_NOISE_PROFILE,
	PROP_NOISE_FILE
};
#define DEF_RELEASE_TIME 0
#define DEF_INTENSITY 20.0f
#define DEF_NOISE_OFFSET 2
#define DEF_WHITENING_FACTOR 0
#define DEF_MASKING 1.0f
#define DEF_TRANSIENT_PROTECTION 0
#define DEF_RESIDUAL FALSE
#define DEF_AUTO_LEARN TRUE
#define DEF_BUILD_NOISE_PROFILE FALSE
#define DEF_NOISE_FILE "noise.fft"

#if 0
/* This means we support signed 16-bit pcm and signed 32-bit pcm in native
* endianness */
#define SUPPORTED_CAPS_STRING \
    GST_AUDIO_CAPS_MAKE("{ " GST_AUDIO_NE(S16) ", " GST_AUDIO_NE(S32) " }")
#endif

/* For simplicity only support 16-bit pcm in native endianness for starters */
//#define SUPPORTED_CAPS_STRING \
    //GST_AUDIO_CAPS_MAKE(GST_AUDIO_NE(S16))
#define SUPPORTED_CAPS_STRING \
    GST_AUDIO_CAPS_MAKE(GST_AUDIO_NE(F32))

/* GObject vmethod implementations */
static void
gst_denoise_class_init(GstDenoiseClass * klass)
{
	GObjectClass *gobject_class;
	GstElementClass *element_class;
	GstBaseTransformClass *btrans_class;
	GstAudioFilterClass *audio_filter_class;
	GstCaps *caps;

	gobject_class = (GObjectClass *)klass;
	element_class = (GstElementClass *)klass;
	btrans_class = (GstBaseTransformClass *)klass;
	audio_filter_class = (GstAudioFilterClass *)klass;

#if 0
	g_object_class_install_property(gobject_class, ARG_METHOD,
		g_param_spec_enum("method", "method", "method",
			GST_TYPE_AUDIOTEMPLATE_METHOD, GST_AUDIOTEMPLATE_METHOD_1,
			G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
#endif

	gobject_class->set_property = gst_denoise_set_property;
	gobject_class->get_property = gst_denoise_get_property;

	// All our properties
	g_object_class_install_property(gobject_class, PROP_RELEASE_TIME,
		g_param_spec_float("release-time", "release time (ms)",
			"How quickly to smooth (ms)", 0, G_MAXFLOAT, DEF_RELEASE_TIME,
			(GParamFlags)(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

	g_object_class_install_property(gobject_class, PROP_INTENSITY,
		g_param_spec_float("intensity", "reduction intensity (db)",
			"How much to reduce the noise in db", 0, G_MAXFLOAT, DEF_INTENSITY,
			(GParamFlags)(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

	g_object_class_install_property(gobject_class, PROP_NOISE_OFFSET,
		g_param_spec_float("noise-offset", "noise offset(db)",
			"How much to scale the noise profile (db)", 0, G_MAXFLOAT, DEF_NOISE_OFFSET,
			(GParamFlags)(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

	g_object_class_install_property(gobject_class, PROP_WHITENING_FACTOR,
		g_param_spec_float("whiten", "whiten amount (%)",
			"How much white noise to add (%)", 0, G_MAXFLOAT, DEF_WHITENING_FACTOR,
			(GParamFlags)(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

	g_object_class_install_property(gobject_class, PROP_MASKING,
		g_param_spec_float("mask", "mask noise",
			"???", 0, G_MAXFLOAT, DEF_MASKING,
			(GParamFlags)(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

	g_object_class_install_property(gobject_class, PROP_TRANSIENT_PROTECTION,
		g_param_spec_float("transient-protection", "transient protection",
			"Multiplier for thresholding onsets with rolling mean", 0, G_MAXFLOAT, DEF_TRANSIENT_PROTECTION,
			(GParamFlags)(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

	g_object_class_install_property(gobject_class, PROP_RESIDUAL,
		g_param_spec_boolean("residual", "residual listen",
			"Listen to residual noise, with signal removed", DEF_RESIDUAL,
			(GParamFlags)(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

	g_object_class_install_property(gobject_class, PROP_AUTO_LEARN,
		g_param_spec_boolean("auto", "auto learn",
			"Automatically learn and apply noise profile", DEF_AUTO_LEARN,
			(GParamFlags)(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

	g_object_class_install_property(gobject_class, PROP_BUILD_NOISE_PROFILE,
		g_param_spec_boolean("build-noise", "build noise profile",
			"Assume all input is noise, and create noise profile accordingly", DEF_BUILD_NOISE_PROFILE,
			(GParamFlags)(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

	g_object_class_install_property(gobject_class, PROP_NOISE_FILE,
		g_param_spec_string("filename", "filename",
			"Filename that the noise profile should be saved/loaded from", DEF_NOISE_FILE,
			(GParamFlags)(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

	/* this function will be called when the format is set before the
	* first buffer comes in, and whenever the format changes */
	audio_filter_class->setup = gst_denoise_setup;

	btrans_class->stop = gst_denoise_stop;

	/* here you set up functions to process data (either in place, or from
	* one input buffer to another output buffer); only one is required */
	//btrans_class->transform = gst_denoise_filter;
	btrans_class->transform_ip = gst_denoise_filter_inplace;
	/* Set some basic metadata about your new element */
	gst_element_class_set_details_simple(element_class,
		"Audio Filter Template", /* FIXME: short name */
		"Filter/Effect/Audio",
		"Filters audio", /* FIXME: short description*/
		"Name <mail@example.com>"); /* FIXME: author */

	caps = gst_caps_from_string(SUPPORTED_CAPS_STRING);
	gst_audio_filter_class_add_pad_templates(audio_filter_class, caps);
	gst_caps_unref(caps);
}

static void
gst_denoise_init(GstDenoise * filter)
{
	GstDenoise *self;
	self = GST_DENOISE(filter);

	self->release = DEF_RELEASE_TIME;
	self->amount_of_reduction = DEF_INTENSITY;
	self->noise_thresholds_offset = DEF_NOISE_OFFSET;
	self->whitening_factor = DEF_WHITENING_FACTOR;
	self->masking = DEF_MASKING;
	self->transient_protection = DEF_TRANSIENT_PROTECTION;
	self->residual_listen = DEF_RESIDUAL;
	self->adaptive_state = DEF_AUTO_LEARN;
	self->noise_learn_state = DEF_BUILD_NOISE_PROFILE;

	free(self->file_location);
	self->file_location = g_strdup(DEF_NOISE_FILE);
	g_print("init\n");
}

static void
gst_denoise_set_property(GObject * object, guint prop_id,
	const GValue * value, GParamSpec * pspec)
{
	GstDenoise *filter = GST_DENOISE(object);

	GST_OBJECT_LOCK(filter);
	switch (prop_id) {
	case PROP_RELEASE_TIME:
		filter->release = g_value_get_float(value);
		break;
	case PROP_INTENSITY:
		filter->amount_of_reduction = g_value_get_float(value);
		break;
	case PROP_NOISE_OFFSET:
		filter->noise_thresholds_offset = g_value_get_float(value);
		break;
	case PROP_WHITENING_FACTOR:
		filter->whitening_factor = g_value_get_float(value);
		break;
	case PROP_MASKING:
		filter->masking = g_value_get_float(value);
		break;
	case PROP_TRANSIENT_PROTECTION:
		filter->transient_protection = g_value_get_float(value);
		break;
	case PROP_RESIDUAL:
		filter->residual_listen = g_value_get_boolean(value);
		break;
	case PROP_AUTO_LEARN:
		filter->adaptive_state = g_value_get_boolean(value);
		break;
	case PROP_BUILD_NOISE_PROFILE:
		filter->noise_learn_state = g_value_get_boolean(value);
		break;
	case PROP_NOISE_FILE:
		g_free(filter->file_location);
		g_print("Set noise file\n");
		filter->file_location = g_value_dup_string(value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
	GST_OBJECT_UNLOCK(filter);
}

static void
gst_denoise_get_property(GObject * object, guint prop_id,
	GValue * value, GParamSpec * pspec)
{
	GstDenoise *filter = GST_DENOISE(object);

	GST_OBJECT_LOCK(filter);
	switch (prop_id) {
	case PROP_RELEASE_TIME:
		g_value_set_float(value, filter->release);
		break;
	case PROP_INTENSITY:
		g_value_set_float(value, filter->amount_of_reduction);
		break;
	case PROP_NOISE_OFFSET:
		g_value_set_float(value, filter->noise_thresholds_offset );
		break;
	case PROP_WHITENING_FACTOR:
		g_value_set_float(value, filter->whitening_factor );
		break;
	case PROP_MASKING:
		g_value_set_float(value, filter->masking );
		break;
	case PROP_TRANSIENT_PROTECTION:
		g_value_set_float(value, filter->transient_protection );
		break;
	case PROP_RESIDUAL:
		g_value_set_boolean(value, filter->residual_listen );
		break;
	case PROP_AUTO_LEARN:
		g_value_set_boolean(value, filter->adaptive_state );
		break;
	case PROP_BUILD_NOISE_PROFILE:
		g_value_set_boolean(value, filter->noise_learn_state );
		break;
	case PROP_NOISE_FILE:
		g_value_set_string(value, filter->file_location );
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
	GST_OBJECT_UNLOCK(filter);
}

static gboolean
gst_denoise_setup(GstAudioFilter * filter,
	const GstAudioInfo * info)
{
	GstDenoise *self;
	GstAudioFormat fmt;
	gint chans, rate;

	self = GST_DENOISE(filter);

	rate = GST_AUDIO_INFO_RATE(info);
	chans = GST_AUDIO_INFO_CHANNELS(info);
	fmt = GST_AUDIO_INFO_FORMAT(info);

	GST_INFO_OBJECT(self, "format %d (%s), rate %d, %d channels",
		fmt, GST_AUDIO_INFO_NAME(info), rate, chans);

	/* if any setup needs to be done (like memory allocated), do it here */

	/* The audio filter base class also saves the audio info in
	* GST_AUDIO_FILTER_INFO(filter) so it's automatically available
	* later from there as well */

	//Sampling related
	self->samp_rate = (float)rate;

	//FFT related
	self->fft_size = FFT_SIZE;
	self->fft_size_2 = self->fft_size/2;
	self->input_fft_buffer = (float*)calloc(self->fft_size, sizeof(float));
	self->output_fft_buffer = (float*)calloc(self->fft_size, sizeof(float));
	self->forward = fftwf_plan_r2r_1d(self->fft_size, self->input_fft_buffer, self->output_fft_buffer, FFTW_R2HC, FFTW_ESTIMATE);
	self->backward = fftwf_plan_r2r_1d(self->fft_size, self->output_fft_buffer, self->input_fft_buffer, FFTW_HC2R, FFTW_ESTIMATE);

	//STFT window related
	self->window_option_input = INPUT_WINDOW;
	self->window_option_output = OUTPUT_WINDOW;
	self->input_window = (float*)calloc(self->fft_size, sizeof(float));
	self->output_window = (float*)calloc(self->fft_size, sizeof(float));

	//buffers for OLA
	self->in_fifo = (float*)calloc(self->fft_size, sizeof(float));
	self->out_fifo = (float*)calloc(self->fft_size, sizeof(float));
	self->output_accum = (float*)calloc(self->fft_size*2, sizeof(float));
	self->overlap_factor = OVERLAP_FACTOR;
	self->hop = (int)(self->fft_size/self->overlap_factor);
	self->input_latency = self->fft_size - self->hop;
	self->read_ptr = self->input_latency; //the initial position because we are that many samples ahead

	//soft bypass
	self->tau = (1.f - expf(-2.f * M_PI * 25.f * 64.f  / self->samp_rate));
	self->wet_dry = 0.f;

	//Arrays for getting bins info
	self->fft_p2 = (float*)calloc((self->fft_size_2+1), sizeof(float));
	self->fft_magnitude = (float*)calloc((self->fft_size_2+1), sizeof(float));
	self->fft_phase = (float*)calloc((self->fft_size_2+1), sizeof(float));

	//noise threshold related
	self->noise_thresholds_p2 = (float*)calloc((self->fft_size_2+1), sizeof(float));
	self->noise_thresholds_scaled = (float*)calloc((self->fft_size_2+1), sizeof(float));
	self->noise_window_count = 0.f;
	self->noise_thresholds_availables = false;

	//noise adaptive estimation related
	self->auto_thresholds = (float*)calloc((self->fft_size_2+1), sizeof(float));
	self->prev_noise_thresholds = (float*)calloc((self->fft_size_2+1), sizeof(float));
	self->s_pow_spec = (float*)calloc((self->fft_size_2+1), sizeof(float));
	self->prev_s_pow_spec = (float*)calloc((self->fft_size_2+1), sizeof(float));
	self->p_min = (float*)calloc((self->fft_size_2+1), sizeof(float));
	self->prev_p_min = (float*)calloc((self->fft_size_2+1), sizeof(float));
	self->speech_p_p = (float*)calloc((self->fft_size_2+1), sizeof(float));
	self->prev_speech_p_p = (float*)calloc((self->fft_size_2+1), sizeof(float));

	//smoothing related
	self->smoothed_spectrum = (float*)calloc((self->fft_size_2+1), sizeof(float));
	self->smoothed_spectrum_prev = (float*)calloc((self->fft_size_2+1), sizeof(float));

	//transient preservation
	self->transient_preserv_prev = (float*)calloc((self->fft_size_2+1), sizeof(float));
	self->tp_window_count = 0.f;
	self->tp_r_mean = 0.f;
	self->transient_present = false;

	//masking related
	self->bark_z = (float*)calloc((self->fft_size_2+1), sizeof(float));
	self->absolute_thresholds = (float*)calloc((self->fft_size_2+1), sizeof(float));
	self->unity_gain_bark_spectrum = (float*)calloc(N_BARK_BANDS, sizeof(float));
	self->spreaded_unity_gain_bark_spectrum = (float*)calloc(N_BARK_BANDS, sizeof(float));
	self->spl_reference_values = (float*)calloc((self->fft_size_2+1), sizeof(float));
	self->alpha_masking = (float*)calloc((self->fft_size_2+1), sizeof(float));
	self->beta_masking = (float*)calloc((self->fft_size_2+1), sizeof(float));
	self->SSF = (float*)calloc((N_BARK_BANDS*N_BARK_BANDS), sizeof(float));
	self->input_fft_buffer_at = (float*)calloc(self->fft_size, sizeof(float));
	self->output_fft_buffer_at = (float*)calloc(self->fft_size, sizeof(float));
	self->forward_at = fftwf_plan_r2r_1d(self->fft_size, self->input_fft_buffer_at, self->output_fft_buffer_at, FFTW_R2HC, FFTW_ESTIMATE);

	//reduction gains related
	self->Gk = (float*)calloc((self->fft_size), sizeof(float));

	//whitening related
	self->residual_max_spectrum = (float*)calloc((self->fft_size), sizeof(float));
	self->max_decay_rate = expf(-1000.f/(((WHITENING_DECAY_RATE) * self->samp_rate)/ self->hop));
	self->whitening_window_count = 0.f;

	//final ensemble related
	self->residual_spectrum = (float*)calloc((self->fft_size), sizeof(float));
	self->denoised_spectrum = (float*)calloc((self->fft_size), sizeof(float));
	self->final_spectrum = (float*)calloc((self->fft_size), sizeof(float));

	//Window combination initialization (pre processing window post processing window)
	fft_pre_and_post_window(self->input_window, self->output_window,
													self->fft_size, self->window_option_input,
													self->window_option_output,	&self->overlap_scale_factor);

	//Set initial gain as unity for the positive part
	initialize_array(self->Gk,1.f,self->fft_size);

	//Compute adaptive initial thresholds
	compute_auto_thresholds(self->auto_thresholds, (float)self->fft_size, (float)self->fft_size_2,
													self->samp_rate);

	//MASKING initializations
	compute_bark_mapping(self->bark_z, self->fft_size_2, (int)self->samp_rate);
	compute_absolute_thresholds(self->absolute_thresholds, self->fft_size_2,
															(int)self->samp_rate);
	spl_reference(self->spl_reference_values, self->fft_size_2, (int)self->samp_rate,
								self->input_fft_buffer_at, self->output_fft_buffer_at,
								&self->forward_at);
	compute_SSF(self->SSF);

	//Initializing unity gain values for offset normalization
	initialize_array(self->unity_gain_bark_spectrum,1.f,N_BARK_BANDS);
	//Convolve unitary energy bark spectrum with SSF
  convolve_with_SSF(self->SSF, self->unity_gain_bark_spectrum,
										self->spreaded_unity_gain_bark_spectrum);

	initialize_array(self->alpha_masking,1.f,self->fft_size_2+1);
	initialize_array(self->beta_masking,0.f,self->fft_size_2+1);

	// Added
	self->enable = 1.f;
	// unused
	self->reset_profile = 0;

	// If configured, load a noise profile from a file
	if (!self->noise_learn_state && !self->adaptive_state){
		g_print("Will load noise data from file:\"%s\"\n", self->file_location);
		FILE *save_file;
		if (0 != fopen_s(&save_file, self->file_location, "r")) {
			g_print("Failed loading noise file\n");
		}
		else {
			const int BUFF_SIZE = 32;
			char str_buff[BUFF_SIZE];
			for (int i = 0; i < self->fft_size_2 + 1; i++) {
				fgets(str_buff, BUFF_SIZE, save_file);
				self->noise_thresholds_p2[i] = (float)atof(str_buff);
				memset(str_buff, 0, sizeof(str_buff));
			}
			self->noise_thresholds_availables = true;
			g_print("Noise print fully loaded from file\n");
		}
	}

	return TRUE;
}

static gboolean
gst_denoise_stop(GstBaseTransform *trans) {
	g_print("Stopping!\n");
	GstDenoise *self = GST_DENOISE(trans);

	// If we have a noise profile, save it to a file
	if (self->noise_learn_state && self->noise_thresholds_availables){
		//get_noise_statistics(self->fft_p2, self->fft_size_2,
			//self->noise_thresholds_p2, self->noise_window_count);
		g_print("Will save noise data to file\n");
		FILE *save_file;
		if (0 != fopen_s(&save_file, self->file_location, "w")) {
			g_print("Failed saving noise file\n");
		}
		else {
			for (int i = 0; i < self->fft_size_2 + 1; i++) {
				gchar* str = g_strdup_printf("%f\n", self->noise_thresholds_p2[i]);
				fputs(str, save_file);
				g_free(str);
			}
			fclose(save_file);
			g_print("Noise print saved\n");
		}
	}

	// Cleanup memory
	g_free(self->file_location);

	free(self->input_fft_buffer);
	free(self->output_fft_buffer);
	fftwf_destroy_plan(self->forward);
	fftwf_destroy_plan(self->backward);

	//STFT window related
	free(self->input_window);
	free(self->output_window);

	//buffers for OLA
	free(self->in_fifo);
	free(self->out_fifo );
	free(self->output_accum );

	//Arrays for getting bins info
	free(self->fft_p2);
	free(self->fft_magnitude);
	free(self->fft_phase);

	//noise threshold related
	free(self->noise_thresholds_p2);
	free(self->noise_thresholds_scaled);

	//noise adaptive estimation related
	free(self->auto_thresholds);
	free(self->prev_noise_thresholds);
	free(self->s_pow_spec);
	free(self->prev_s_pow_spec);
	free(self->p_min);
	free(self->prev_p_min);
	free(self->speech_p_p);
	free(self->prev_speech_p_p);

	//smoothing related
	free(self->smoothed_spectrum);
	free(self->smoothed_spectrum_prev);

	//transient preservation
	free(self->transient_preserv_prev);

	//masking related
	free(self->bark_z);
	free(self->absolute_thresholds);
	free(self->unity_gain_bark_spectrum);
	free(self->spreaded_unity_gain_bark_spectrum);
	free(self->spl_reference_values);
	free(self->alpha_masking);
	free(self->beta_masking);
	free(self->SSF);
	free(self->input_fft_buffer_at);
	free(self->output_fft_buffer_at);
	fftwf_destroy_plan(self->forward_at);

	//reduction gains related
	free(self->Gk);

	//whitening related
	free(self->residual_max_spectrum);

	//final ensemble related
	free(self->residual_spectrum);
	free(self->denoised_spectrum);
	free(self->final_spectrum);

	return true;
}

/* You may choose to implement either a copying filter or an
* in-place filter (or both).  Implementing only one will give
* full functionality, however, implementing both will cause
* audiofilter to use the optimal function in every situation,
* with a minimum of memory copies. */

static GstFlowReturn
gst_denoise_filter(GstBaseTransform * base_transform,
	GstBuffer * inbuf, GstBuffer * outbuf)
{
	GstDenoise *filter = GST_DENOISE(base_transform);
	GstMapInfo map_in;
	GstMapInfo map_out;

	GST_LOG_OBJECT(filter, "transform buffer");
	g_print("NOT ip\n");

	/* FIXME: do something interesting here.  We simply copy the input data
	* to the output buffer for now. */
	if (gst_buffer_map(inbuf, &map_in, GST_MAP_READ)) {
		if (gst_buffer_map(outbuf, &map_out, GST_MAP_WRITE)) {
			g_assert(map_out.size == map_in.size);
			memcpy(map_out.data, map_in.data, map_out.size);
			gst_buffer_unmap(outbuf, &map_out);
		}
		gst_buffer_unmap(inbuf, &map_in);
	}

	return GST_FLOW_OK;
}

static GstFlowReturn
gst_denoise_filter_inplace(GstBaseTransform * base_transform,
	GstBuffer * buf)
{
	GstDenoise *self = GST_DENOISE(base_transform);
	GstFlowReturn flow = GST_FLOW_OK;
	GstMapInfo map;

	GST_LOG_OBJECT(self, "transform buffer in place");

	//Softbypass targets in case of disabled or enabled
	if(self->enable == 0.f)
	{ //if disabled
		self->wet_dry_target = 0.f;
	}
	else
	{ //if enabled
		self->wet_dry_target = 1.f;
	}
	//Interpolate parameters over time softly to bypass without clicks or pops
	self->wet_dry += self->tau * (self->wet_dry_target - self->wet_dry) + FLT_MIN;

	//Parameters values
	/*exponential decay coefficients for envelopes and adaptive noise profiling
		These must take into account the hop size as explained in the following paper
		FFT-BASED DYNAMIC RANGE COMPRESSION*/
	if (self->release != 0.f) //This allows to turn off smoothing with 0 ms in order to use masking only
	{
		self->release_coeff = expf(-1000.f/(((self->release) * self->samp_rate)/ self->hop));
	}
	else
	{
		self->release_coeff = 0.f; //This avoids incorrect results when moving sliders rapidly
	}

	self->amount_of_reduction_linear = from_dB(-1.f * self->amount_of_reduction);
	self->thresholds_offset_linear = from_dB(self->noise_thresholds_offset);
	self->whitening_factor = self->whitening_factor_pc / 100.f;

	if (!gst_buffer_map(buf, &map, GST_MAP_READWRITE)) {
		g_print("Failed mapping!\n");
		return GST_FLOW_ERROR;
	}
	gint n_samples = 0;
	gint k = 0;
	float *samples = (float*) map.data;
	switch (GST_AUDIO_FILTER_FORMAT(self)) {
	//case GST_AUDIO_FORMAT_S16LE:
	//case GST_AUDIO_FORMAT_S16BE: {
	case GST_AUDIO_FORMAT_F32LE: {
		//gint16 *samples = (gint16*) map.data;
		n_samples = (gint)map.size / sizeof(float);
		//g_print("Recv %i\n", n_samples);

		//main loop for processing
		for (int pos = 0; pos < n_samples; pos++)
		{
			//Store samples int the input buffer
			//self->in_fifo[self->read_ptr] = self->input[pos];
			self->in_fifo[self->read_ptr] = samples[pos];
			//Output samples in the output buffer (even zeros introduced by latency)
			//self->output[pos] = self->out_fifo[self->read_ptr - self->input_latency];
			samples[pos] = self->out_fifo[self->read_ptr - self->input_latency];
			//Now move the read pointer
			self->read_ptr++;

			//Once the buffer is full we can do stuff
			if (self->read_ptr < self->fft_size)
				continue;
			//g_print("full\n");
			//Reset the input buffer position
			self->read_ptr = self->input_latency;

			//----------STFT Analysis------------

			//Adding and windowing the frame input values in the center (zero-phasing)
			for (k = 0; k < self->fft_size; k++)
			{
				self->input_fft_buffer[k] = self->in_fifo[k] * self->input_window[k];
			}

			//----------FFT Analysis------------

			//Do transform
			fftwf_execute(self->forward);

			//-----------GET INFO FROM BINS--------------

			get_info_from_bins(self->fft_p2, self->fft_magnitude, self->fft_phase,
				self->fft_size_2, self->fft_size,
				self->output_fft_buffer);

			/////////////////////SPECTRAL PROCESSING//////////////////////////

			/*This section countains the specific noise reduction processing blocks
				but it could be replaced with any spectral processing (I'm looking at you future tinkerer)
				Parameters for the STFT transform can be changed at the top of this file
			*/

			//If the spectrum is not silence
			if (!is_empty(self->fft_p2, self->fft_size_2))
			{
				//If adaptive noise is selected the noise is adapted in time
				if (self->adaptive_state)
				{
					//This has to be revised(issue 8 on github)
					adapt_noise(self->fft_p2, self->fft_size_2, self->noise_thresholds_p2,
						self->auto_thresholds, self->prev_noise_thresholds,
						self->s_pow_spec, self->prev_s_pow_spec, self->p_min,
						self->prev_p_min, self->speech_p_p, self->prev_speech_p_p);

					self->noise_thresholds_availables = true;
				}

				/*If selected estimate noise spectrum is based on selected portion of signal
				 *do not process the signal
				 */
				if (self->noise_learn_state)
				{ //MANUAL

					//Increase window count for rolling mean
					self->noise_window_count++;

					get_noise_statistics(self->fft_p2, self->fft_size_2,
						self->noise_thresholds_p2, self->noise_window_count);

					self->noise_thresholds_availables = true;
				}
				else
				{
					//If there is a noise profile reduce noise
					if (self->noise_thresholds_availables == true)
					{
						//Detector smoothing and oversubtraction
						preprocessing(self->thresholds_offset_linear, self->fft_p2,
							self->noise_thresholds_p2, self->noise_thresholds_scaled,
							self->smoothed_spectrum, self->smoothed_spectrum_prev,
							self->fft_size_2, self->bark_z, self->absolute_thresholds,
							self->SSF, self->release_coeff,
							self->spreaded_unity_gain_bark_spectrum,
							self->spl_reference_values, self->alpha_masking,
							self->beta_masking, self->masking, self->adaptive_state,
							self->amount_of_reduction_linear, self->transient_preserv_prev,
							&self->tp_window_count, &self->tp_r_mean,
							&self->transient_present, self->transient_protection);

						//Supression rule
						spectral_gain(self->fft_p2, self->noise_thresholds_p2,
							self->noise_thresholds_scaled, self->smoothed_spectrum,
							self->fft_size_2, self->adaptive_state, self->Gk,
							self->transient_protection, self->transient_present);

						//apply gains
						denoised_calulation(self->fft_size, self->output_fft_buffer,
							self->denoised_spectrum, self->Gk);

						//residual signal
						residual_calulation(self->fft_size, self->output_fft_buffer,
							self->residual_spectrum, self->denoised_spectrum,
							self->whitening_factor, self->residual_max_spectrum,
							&self->whitening_window_count, self->max_decay_rate);

						//Ensemble the final spectrum using residual and denoised
						final_spectrum_ensemble(self->fft_size, self->final_spectrum,
							self->residual_spectrum,
							self->denoised_spectrum,
							self->amount_of_reduction_linear,
							self->residual_listen);

						soft_bypass(self->final_spectrum, self->output_fft_buffer,
							self->wet_dry, self->fft_size);
					}
				}
			}

			///////////////////////////////////////////////////////////

			//----------STFT Synthesis------------

			//------------FFT Synthesis-------------

			//Do inverse transform
			fftwf_execute(self->backward);

			//Normalizing value
			for (k = 0; k < self->fft_size; k++)
			{
				self->input_fft_buffer[k] = self->input_fft_buffer[k] / self->fft_size;
			}

			//------------OVERLAPADD-------------

			//Windowing scaling and accumulation
			for (k = 0; k < self->fft_size; k++)
			{
				self->output_accum[k] += (self->output_window[k] * self->input_fft_buffer[k]) / (self->overlap_scale_factor * self->overlap_factor);
			}

			//Output samples up to the hop size
			for (k = 0; k < self->hop; k++)
			{
				self->out_fifo[k] = self->output_accum[k];
			}

			//shift FFT accumulator the hop size
			memmove(self->output_accum, self->output_accum + self->hop,
				self->fft_size * sizeof(float));

			//move input FIFO
			for (k = 0; k < self->input_latency; k++)
			{
				self->in_fifo[k] = self->in_fifo[k + self->hop];
			}
			//-------------------------------
		}
		//g_print("Latency = %i\n", self->input_latency);
		break;
	}
	default:
		g_warning("Unexpected audio format %s!",
			GST_AUDIO_INFO_NAME(GST_AUDIO_FILTER_INFO(self)));
		g_print("format err!\n");
		flow = GST_FLOW_ERROR;
		break;
	}
	gst_buffer_unmap(buf, &map);
	// Account for added latency
	GstClockTime was = buf->pts;
	GstClockTime latency_time = (self->input_latency / self->samp_rate) * GST_SECOND;
	buf->pts = buf->pts > latency_time ? buf->pts - latency_time : buf->pts;
	GST_DEBUG("Latency %" GST_TIME_FORMAT " Time was %" GST_TIME_FORMAT " now %" GST_TIME_FORMAT " \n",
		GST_TIME_ARGS(latency_time), GST_TIME_ARGS(was), GST_TIME_ARGS(buf->pts));

	return flow;
}

static gboolean
plugin_init(GstPlugin * plugin)
{
	/* Register debug category for filtering log messages */
	GST_DEBUG_CATEGORY_INIT(denoise_debug, "denoise", 0,
		"Noise Remover");

	/* This is the name used in gst-launch-1.0 and gst_element_factory_make() */
	return gst_element_register(plugin, "denoise", GST_RANK_NONE,
		GST_TYPE_DENOISE);
}

/* gstreamer looks for this structure to register plugins
*
*/
#ifndef PACKAGE
#define PACKAGE "myfirstplugin"
#endif
#define VERSION "0"

GST_PLUGIN_DEFINE(
	GST_VERSION_MAJOR,
	GST_VERSION_MINOR,
	denoise,
	"Noise Remover",
	plugin_init,
	VERSION,
	"Proprietary",
	"GStreamer",
	"http://gstreamer.freedesktop.org"
);
