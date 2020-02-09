#include "oboe_stream/oboe_stream.h"
#include <stdio.h>
#include "z_jni_shared.c"

static OBOE_STREAM *streamPtr = NULL;

static void process_callback(void *context, int sRate, int bufFrames,
    int nIn, const float *inBuf, int nOut, float *outBuf) {
  pthread_mutex_lock(&mutex);
  libpd_process_float(bufFrames / libpd_blocksize(), inBuf, outBuf);
  pthread_mutex_unlock(&mutex);
}

JNIEXPORT jstring JNICALL Java_org_puredata_core_PdBase_audioImplementation
(JNIEnv *env , jclass cls) {
  return (*env)->NewStringUTF(env, "Oboe");
}

JNIEXPORT jboolean JNICALL Java_org_puredata_core_PdBase_implementsAudio
(JNIEnv *env, jclass cls) {
  return 1;
}

JNIEXPORT jint JNICALL Java_org_puredata_core_PdBase_openAudio
(JNIEnv *env, jclass cls, jint inChans, jint outChans, jint sRate,
jobject options) {
  Java_org_puredata_core_PdBase_closeAudio(env, cls);
  pthread_mutex_lock(&mutex);
  jint err = libpd_init_audio(inChans, outChans, sRate);
  pthread_mutex_unlock(&mutex);
  if (err) return err;
  streamPtr = oboe_open(sRate, inChans, outChans, libpd_blocksize(),
      process_callback, NULL);
  return !streamPtr;
}

JNIEXPORT void JNICALL Java_org_puredata_core_PdBase_closeAudio
(JNIEnv *env, jclass cls) {
  if (streamPtr) {
    oboe_close(streamPtr);
    streamPtr = NULL;
  }
}

JNIEXPORT jint JNICALL Java_org_puredata_core_PdBase_startAudio
(JNIEnv *env, jclass cls) {
  return streamPtr ? oboe_start(streamPtr) : -1;
}

JNIEXPORT jint JNICALL Java_org_puredata_core_PdBase_pauseAudio
(JNIEnv *env, jclass cls) {
  if (streamPtr) {
    oboe_pause(streamPtr);
    return 0;
  } else {
    return -1;
  }
}

JNIEXPORT jboolean JNICALL Java_org_puredata_core_PdBase_isRunning
(JNIEnv *env, jclass cls) {
  return streamPtr && oboe_is_running(streamPtr);
}

JNIEXPORT jint JNICALL Java_org_puredata_core_PdBase_suggestSampleRate
(JNIEnv *env, jclass cls) {
  return -1;
}

JNIEXPORT jint JNICALL Java_org_puredata_core_PdBase_suggestInputChannels
(JNIEnv *env, jclass cls) {
  return -1;
}

JNIEXPORT jint JNICALL Java_org_puredata_core_PdBase_suggestOutputChannels
(JNIEnv *env, jclass cls) {
  return -1;
}
