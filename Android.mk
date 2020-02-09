LOCAL_PATH := $(call my-dir)

# PD-specific flags

PD_SRC_FILES := \
  pure-data/src/d_arithmetic.c pure-data/src/d_array.c pure-data/src/d_ctl.c \
  pure-data/src/d_dac.c pure-data/src/d_delay.c pure-data/src/d_fft.c \
  pure-data/src/d_fft_fftsg.c \
  pure-data/src/d_filter.c pure-data/src/d_global.c pure-data/src/d_math.c \
  pure-data/src/d_misc.c pure-data/src/d_osc.c pure-data/src/d_resample.c \
  pure-data/src/d_soundfile.c pure-data/src/d_ugen.c \
  pure-data/src/g_all_guis.c pure-data/src/g_array.c pure-data/src/g_bang.c \
  pure-data/src/g_canvas.c pure-data/src/g_clone.c pure-data/src/g_editor.c \
  pure-data/src/g_graph.c pure-data/src/g_guiconnect.c pure-data/src/g_hdial.c \
  pure-data/src/g_hslider.c pure-data/src/g_io.c pure-data/src/g_mycanvas.c \
  pure-data/src/g_numbox.c pure-data/src/g_readwrite.c \
  pure-data/src/g_rtext.c pure-data/src/g_scalar.c pure-data/src/g_template.c \
  pure-data/src/g_text.c pure-data/src/g_toggle.c pure-data/src/g_traversal.c \
  pure-data/src/g_vdial.c pure-data/src/g_vslider.c pure-data/src/g_vumeter.c \
  pure-data/src/m_atom.c pure-data/src/m_binbuf.c pure-data/src/m_class.c \
  pure-data/src/m_conf.c pure-data/src/m_glob.c pure-data/src/m_memory.c \
  pure-data/src/m_obj.c pure-data/src/m_pd.c pure-data/src/m_sched.c \
  pure-data/src/s_audio.c pure-data/src/s_audio_dummy.c \
  pure-data/src/s_inter.c \
  pure-data/src/s_loader.c pure-data/src/s_main.c pure-data/src/s_path.c \
  pure-data/src/s_print.c pure-data/src/s_utf8.c pure-data/src/x_acoustics.c \
  pure-data/src/x_arithmetic.c pure-data/src/x_connective.c \
  pure-data/src/x_gui.c pure-data/src/x_list.c pure-data/src/x_midi.c \
  pure-data/src/x_misc.c pure-data/src/x_net.c pure-data/src/x_array.c \
  pure-data/src/x_time.c pure-data/src/x_interface.c pure-data/src/x_scalar.c \
  pure-data/src/x_text.c pure-data/src/x_vexp.c pure-data/src/x_vexp_if.c \
  pure-data/src/x_vexp_fun.c libpd_wrapper/s_libpdmidi.c \
  libpd_wrapper/x_libpdreceive.c libpd_wrapper/z_libpd.c \
  libpd_wrapper/util/ringbuffer.c libpd_wrapper/util/z_queued.c \
  libpd_wrapper/z_hooks.c libpd_wrapper/util/z_print_util.c
PD_C_INCLUDES := $(LOCAL_PATH)/pure-data/src $(LOCAL_PATH)/libpd_wrapper \
  $(LOCAL_PATH)/libpd_wrapper/util
PD_CFLAGS := -DPD -DHAVE_UNISTD_H -DHAVE_LIBDL -DUSEAPI_DUMMY -w
PD_JNI_CFLAGS := -Wno-int-to-pointer-cast -Wno-pointer-to-int-cast
PD_LDLIBS := -ldl -latomic

# Build libpd

include $(CLEAR_VARS)

LOCAL_MODULE := pd
LOCAL_C_INCLUDES := $(PD_C_INCLUDES)
LOCAL_CFLAGS := $(PD_CFLAGS)
LOCAL_LDLIBS := $(PD_LDLIBS)
LOCAL_SRC_FILES := $(PD_SRC_FILES)
include $(BUILD_SHARED_LIBRARY)

# Build OpenSL JNI binary

include $(CLEAR_VARS)

LOCAL_MODULE := pdnativeopensl
LOCAL_C_INCLUDES := $(PD_C_INCLUDES) $(LOCAL_PATH)/jni
LOCAL_CFLAGS := $(PD_JNI_CFLAGS)
LOCAL_LDLIBS := -lOpenSLES -llog
LOCAL_SRC_FILES := jni/opensl_stream/opensl_stream.c jni/z_jni_opensl.c
LOCAL_SHARED_LIBRARIES := pd
include $(BUILD_SHARED_LIBRARY)

# Build plain JNI binary

include $(CLEAR_VARS)

LOCAL_MODULE := pdnative
LOCAL_C_INCLUDES := $(PD_C_INCLUDES)
LOCAL_CFLAGS := $(PD_JNI_CFLAGS)
LOCAL_SRC_FILES := jni/z_jni_plain.c
LOCAL_SHARED_LIBRARIES := pd
include $(BUILD_SHARED_LIBRARY)


# OBOE-specific flags

OBOE_SRC_FILES := \
    oboe/src/aaudio/AAudioLoader.cpp \
    oboe/src/aaudio/AudioStreamAAudio.cpp \
    oboe/src/common/AudioSourceCaller.cpp \
    oboe/src/common/AudioStream.cpp \
    oboe/src/common/AudioStreamBuilder.cpp \
    oboe/src/common/DataConversionFlowGraph.cpp \
    oboe/src/common/FilterAudioStream.cpp \
    oboe/src/common/FixedBlockAdapter.cpp \
    oboe/src/common/FixedBlockReader.cpp \
    oboe/src/common/FixedBlockWriter.cpp \
    oboe/src/common/LatencyTuner.cpp \
    oboe/src/common/SourceFloatCaller.cpp \
    oboe/src/common/SourceI16Caller.cpp \
    oboe/src/common/Utilities.cpp \
    oboe/src/common/QuirksManager.cpp \
    oboe/src/fifo/FifoBuffer.cpp \
    oboe/src/fifo/FifoController.cpp \
    oboe/src/fifo/FifoControllerBase.cpp \
    oboe/src/fifo/FifoControllerIndirect.cpp \
    oboe/src/flowgraph/FlowGraphNode.cpp \
    oboe/src/flowgraph/ClipToRange.cpp \
    oboe/src/flowgraph/ManyToMultiConverter.cpp \
    oboe/src/flowgraph/MonoToMultiConverter.cpp \
    oboe/src/flowgraph/RampLinear.cpp \
    oboe/src/flowgraph/SampleRateConverter.cpp \
    oboe/src/flowgraph/SinkFloat.cpp \
    oboe/src/flowgraph/SinkI16.cpp \
    oboe/src/flowgraph/SinkI24.cpp \
    oboe/src/flowgraph/SourceFloat.cpp \
    oboe/src/flowgraph/SourceI16.cpp \
    oboe/src/flowgraph/SourceI24.cpp \
    oboe/src/flowgraph/resampler/IntegerRatio.cpp \
    oboe/src/flowgraph/resampler/LinearResampler.cpp \
    oboe/src/flowgraph/resampler/MultiChannelResampler.cpp \
    oboe/src/flowgraph/resampler/PolyphaseResampler.cpp \
    oboe/src/flowgraph/resampler/PolyphaseResamplerMono.cpp \
    oboe/src/flowgraph/resampler/PolyphaseResamplerStereo.cpp \
    oboe/src/flowgraph/resampler/SincResampler.cpp \
    oboe/src/flowgraph/resampler/SincResamplerStereo.cpp \
    oboe/src/opensles/AudioInputStreamOpenSLES.cpp \
    oboe/src/opensles/AudioOutputStreamOpenSLES.cpp \
    oboe/src/opensles/AudioStreamBuffered.cpp \
    oboe/src/opensles/AudioStreamOpenSLES.cpp \
    oboe/src/opensles/EngineOpenSLES.cpp \
    oboe/src/opensles/OpenSLESUtilities.cpp \
    oboe/src/opensles/OutputMixerOpenSLES.cpp \
    oboe/src/common/StabilizedCallback.cpp \
    oboe/src/common/Trace.cpp \
    oboe/src/common/Version.cpp
OBOE_C_INCLUDES := $(LOCAL_PATH)/oboe/src $(LOCAL_PATH)/oboe/include \
OBOE_CPPFLAGS := -Wall -Wextra-semi -Wshadow -Wshadow-field -Ofast
OBOE_LDLIBS := -llog -lOpenSLES

OBOE_STREAM_SRC_FILES := \
    jni/oboe_stream/oboe_stream.cpp  \
    jni/z_jni_oboe.c  \
    oboe/samples/LiveEffect/src/main/cpp/FullDuplexStream.cpp
OBOE_STREAM_INCLUDES := \
    $(PD_C_INCLUDES) \
    $(LOCAL_PATH)/jni \
    $(OBOE_C_INCLUDES) \
    $(LOCAL_PATH)/oboe/samples/LiveEffect/src/main/cpp

# Build oboe

include $(CLEAR_VARS)

LOCAL_MODULE := oboe
LOCAL_C_INCLUDES := $(OBOE_C_INCLUDES)
LOCAL_CPPFLAGS := $(OBOE_CPPFLAGS)
LOCAL_LDLIBS := $(OBOE_LDLIBS)
LOCAL_SRC_FILES := $(OBOE_SRC_FILES)
include $(BUILD_SHARED_LIBRARY)

# Build Oboe JNI binary

include $(CLEAR_VARS)

LOCAL_MODULE := pdnativeoboe
LOCAL_C_INCLUDES := $(OBOE_STREAM_INCLUDES)
LOCAL_CFLAGS := $(PD_JNI_CFLAGS)
LOCAL_LDLIBS := -llog
LOCAL_SRC_FILES := $(OBOE_STREAM_SRC_FILES)
LOCAL_SHARED_LIBRARIES := pd oboe
include $(BUILD_SHARED_LIBRARY)


# Build libchoice.so

include $(CLEAR_VARS)

LOCAL_MODULE := choice
LOCAL_C_INCLUDES := $(LOCAL_PATH)/pure-data/src
LOCAL_CFLAGS := -DPD
LOCAL_SRC_FILES := pure-data/extra/choice/choice.c
LOCAL_SHARED_LIBRARIES := pd

include $(BUILD_SHARED_LIBRARY)


# Build libbonk_tilde.so

include $(CLEAR_VARS)

LOCAL_MODULE := bonk_tilde
LOCAL_C_INCLUDES := $(LOCAL_PATH)/pure-data/src
LOCAL_CFLAGS := -DPD
LOCAL_SRC_FILES := pure-data/extra/bonk~/bonk~.c
LOCAL_SHARED_LIBRARIES := pd

include $(BUILD_SHARED_LIBRARY)


# Build liblrshift_tilde.so

include $(CLEAR_VARS)

LOCAL_MODULE := lrshift_tilde
LOCAL_C_INCLUDES := $(LOCAL_PATH)/pure-data/src
LOCAL_CFLAGS := -DPD
LOCAL_SRC_FILES := pure-data/extra/lrshift~/lrshift~.c
LOCAL_SHARED_LIBRARIES := pd

include $(BUILD_SHARED_LIBRARY)


# Build libfiddle_tilde.so

include $(CLEAR_VARS)

LOCAL_MODULE := fiddle_tilde
LOCAL_C_INCLUDES := $(LOCAL_PATH)/pure-data/src
LOCAL_CFLAGS := -DPD
LOCAL_SRC_FILES := pure-data/extra/fiddle~/fiddle~.c
LOCAL_SHARED_LIBRARIES := pd

include $(BUILD_SHARED_LIBRARY)


# Build libsigmund_tilde.so

include $(CLEAR_VARS)

LOCAL_MODULE := sigmund_tilde
LOCAL_C_INCLUDES := $(LOCAL_PATH)/pure-data/src
LOCAL_CFLAGS := -DPD
LOCAL_SRC_FILES := pure-data/extra/sigmund~/sigmund~.c
LOCAL_SHARED_LIBRARIES := pd

include $(BUILD_SHARED_LIBRARY)


# Build libpique.so

include $(CLEAR_VARS)

LOCAL_MODULE := pique
LOCAL_C_INCLUDES := $(LOCAL_PATH)/pure-data/src
LOCAL_CFLAGS := -DPD
LOCAL_SRC_FILES := pure-data/extra/pique/pique.c
LOCAL_SHARED_LIBRARIES := pd

include $(BUILD_SHARED_LIBRARY)


# Build libloop_tilde.so

include $(CLEAR_VARS)

LOCAL_MODULE := loop_tilde
LOCAL_C_INCLUDES := $(LOCAL_PATH)/pure-data/src
LOCAL_CFLAGS := -DPD
LOCAL_SRC_FILES := pure-data/extra/loop~/loop~.c
LOCAL_SHARED_LIBRARIES := pd

include $(BUILD_SHARED_LIBRARY)


# Build bob_tilde.so

include $(CLEAR_VARS)

LOCAL_MODULE := bob_tilde
LOCAL_C_INCLUDES := $(LOCAL_PATH)/pure-data/src
LOCAL_CFLAGS := -DPD
LOCAL_SRC_FILES := pure-data/extra/bob~/bob~.c
LOCAL_SHARED_LIBRARIES := pd

include $(BUILD_SHARED_LIBRARY)


# Build stdout.so

include $(CLEAR_VARS)

LOCAL_MODULE := stdout
LOCAL_C_INCLUDES := $(LOCAL_PATH)/pure-data/src
LOCAL_CFLAGS := -DPD
LOCAL_SRC_FILES := pure-data/extra/stdout/stdout.c
LOCAL_SHARED_LIBRARIES := pd

include $(BUILD_SHARED_LIBRARY)

