/* GStreamer audio filter example class
* Copyright (C) <1999> Erik Walthinsen <omega@cse.ogi.edu>
* Copyright (C) <2003> David Schleef <ds@schleef.org>
* Copyright (C) YEAR AUTHOR_NAME AUTHOR_EMAIL
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
* SECTION:element-plugin
*
* FIXME:Describe plugin here.
*
* <refsect2>
* <title>Example launch line</title>
* |[
* gst-launch -v -m audiotestsrc ! plugin ! autoaudiosink
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
	ARG_0
	/* FILL ME */
};

#if 0
/* This means we support signed 16-bit pcm and signed 32-bit pcm in native
* endianness */
#define SUPPORTED_CAPS_STRING \
    GST_AUDIO_CAPS_MAKE("{ " GST_AUDIO_NE(S16) ", " GST_AUDIO_NE(S32) " }")
#endif

/* For simplicity only support 16-bit pcm in native endianness for starters */
#define SUPPORTED_CAPS_STRING \
    GST_AUDIO_CAPS_MAKE(GST_AUDIO_NE(S16))

/* GObject vmethod implementations */
static void
gst_audio_filter_template_class_init(GstAudioFilterTemplateClass * klass)
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

	gobject_class->set_property = gst_audio_filter_template_set_property;
	gobject_class->get_property = gst_audio_filter_template_get_property;

	/* this function will be called when the format is set before the
	* first buffer comes in, and whenever the format changes */
	audio_filter_class->setup = gst_audio_filter_template_setup;

	/* here you set up functions to process data (either in place, or from
	* one input buffer to another output buffer); only one is required */
	//btrans_class->transform = gst_audio_filter_template_filter;
	btrans_class->transform_ip = gst_audio_filter_template_filter_inplace;
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
gst_audio_filter_template_init(GstAudioFilterTemplate * filter)
{
	/* This function is called when a new filter object is created. You
	* would typically do things like initialise properties to their
	* default values here if needed. */
}

static void
gst_audio_filter_template_set_property(GObject * object, guint prop_id,
	const GValue * value, GParamSpec * pspec)
{
	GstAudioFilterTemplate *filter = GST_AUDIO_FILTER_TEMPLATE(object);

	GST_OBJECT_LOCK(filter);
	switch (prop_id) {
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
	GST_OBJECT_UNLOCK(filter);
}

static void
gst_audio_filter_template_get_property(GObject * object, guint prop_id,
	GValue * value, GParamSpec * pspec)
{
	GstAudioFilterTemplate *filter = GST_AUDIO_FILTER_TEMPLATE(object);

	GST_OBJECT_LOCK(filter);
	switch (prop_id) {
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
	GST_OBJECT_UNLOCK(filter);
}

static gboolean
gst_audio_filter_template_setup(GstAudioFilter * filter,
	const GstAudioInfo * info)
{
	GstAudioFilterTemplate *self;
	GstAudioFormat fmt;
	gint chans, rate;

	self = GST_AUDIO_FILTER_TEMPLATE(filter);

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
	self->hop = self->fft_size/self->overlap_factor;
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
	compute_auto_thresholds(self->auto_thresholds, self->fft_size, self->fft_size_2,
													self->samp_rate);

	//MASKING initializations
	compute_bark_mapping(self->bark_z, self->fft_size_2, self->samp_rate);
	compute_absolute_thresholds(self->absolute_thresholds, self->fft_size_2,
															self->samp_rate);
	spl_reference(self->spl_reference_values, self->fft_size_2, self->samp_rate,
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

	return TRUE;
}

/* You may choose to implement either a copying filter or an
* in-place filter (or both).  Implementing only one will give
* full functionality, however, implementing both will cause
* audiofilter to use the optimal function in every situation,
* with a minimum of memory copies. */

static GstFlowReturn
gst_audio_filter_template_filter(GstBaseTransform * base_transform,
	GstBuffer * inbuf, GstBuffer * outbuf)
{
	GstAudioFilterTemplate *filter = GST_AUDIO_FILTER_TEMPLATE(base_transform);
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
gst_audio_filter_template_filter_inplace(GstBaseTransform * base_transform,
	GstBuffer * buf)
{
	GstAudioFilterTemplate *filter = GST_AUDIO_FILTER_TEMPLATE(base_transform);
	GstFlowReturn flow = GST_FLOW_OK;
	GstMapInfo map;

	GST_LOG_OBJECT(filter, "transform buffer in place");
	//g_print("ip\n");

	/* FIXME: do something interesting here.  Doing nothing means the input
	* buffer is simply pushed out as is without any modification */
	if (gst_buffer_map(buf, &map, GST_MAP_READWRITE)) {
#if 0
		switch (GST_AUDIO_FILTER_FORMAT(filter)) {
		case GST_AUDIO_FORMAT_S16LE:
		case GST_AUDIO_FORMAT_S16BE: {
			gint16 *samples = map.data;
			guint n_samples = map.size / sizeof(gint16);
			guint i;

			for (i = 0; i < n; ++n) {
				samples[i] = samples[i];
			}
			break;
		}
		default:
			g_warning("Unexpected audio format %s!",
				GST_AUDIO_INFO_NAME(GST_AUDIO_FILTER_INFO(filter)));
			flow = GST_FLOW_ERROR;
			break;
		}
#endif
		gst_buffer_unmap(buf, &map);
	}

	return flow;
}

static gboolean
plugin_init(GstPlugin * plugin)
{
	/* Register debug category for filtering log messages
	* FIXME:exchange the string 'Template plugin' with your description */
	GST_DEBUG_CATEGORY_INIT(audiofiltertemplate_debug, "audiofiltertemplate", 0,
		"Audio filter template example");

	/* This is the name used in gst-launch-1.0 and gst_element_factory_make() */
	return gst_element_register(plugin, "audiofiltertemplate", GST_RANK_NONE,
		GST_TYPE_AUDIO_FILTER_TEMPLATE);
}

/* gstreamer looks for this structure to register plugins
*
* FIXME:exchange the string 'Template plugin' with you plugin description
*/
#ifndef PACKAGE
#define PACKAGE "myfirstplugin"
#endif
#define VERSION "0"

GST_PLUGIN_DEFINE(
	GST_VERSION_MAJOR,
	GST_VERSION_MINOR,
	audiofiltertemplateplugin,
	"Audio filter template plugin",
	plugin_init,
	VERSION,
	"Proprietary",
	"GStreamer",
	"http://gstreamer.freedesktop.org"
);
