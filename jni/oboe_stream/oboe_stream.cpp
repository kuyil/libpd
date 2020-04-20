/*
 * Based on sample code by Victor Lazzarini, available at
 * http://audioprograming.wordpress.com/2012/03/03/android-audio-streaming-with-opensl-es-and-the-ndk/
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "oboe_stream.h"

#include <android/log.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <memory>

#include <oboe/Oboe.h>
#include <FullDuplexStream.h>

using namespace oboe;

#ifdef __ANDROID__
#include <android/log.h>

#if !defined(ALOG)

// FOR TESTING ONLY: Always set this to 0 in production
#define ENABLE_ANDROID_LOGGING_EVEN_IN_RELEASE 0

// NDEBUG --> release
#if defined(NDEBUG) && ENABLE_ANDROID_LOGGING_EVEN_IN_RELEASE == 0
#define ALOG(...)
#else
#define ALOG __android_log_print
#endif

#define ALOGD(...) ALOG(ANDROID_LOG_DEBUG, "oboe_stream",  __VA_ARGS__)
#define ALOGI(...) ALOG(ANDROID_LOG_INFO, "oboe_stream",  __VA_ARGS__)
#define ALOGW(...) ALOG(ANDROID_LOG_WARN, "oboe_stream",  __VA_ARGS__)

// Why are these not defined in terms of ALOG?
// ERROR and FATAL level log entries should appear even in release.
#define ALOGE(...) __android_log_print(ANDROID_LOG_ERROR, "oboe_stream", __VA_ARGS__)
#define ALOGF(...) __android_log_print(ANDROID_LOG_FATAL, "oboe_stream", __VA_ARGS__)

#endif //ALOG
#endif // __ANDROID__

#define BLOCKS_PER_CALLBACK 16

void logStreamConfig(oboe::ManagedStream& stream) {
    switch (stream->getDirection()) {
        case oboe::Direction::Input:
            ALOGD("direction = INPUT");
            break;
        case oboe::Direction::Output:
            ALOGD("direction = OUTPUT");
            break;
    }

    ALOGD("audio api = %s", stream->usesAAudio() ? "AAudio" : "OpenSL");
    ALOGD("sample rate = %d", stream->getSampleRate());
    ALOGD("channels = %d", stream->getChannelCount());

    switch (stream->getFormat()) {
        case oboe::AudioFormat::I16:
            ALOGD("format = %s", "I16");
            break;
        case oboe::AudioFormat::Float:
            ALOGD("format = %s", "Float");
            break;
        case oboe::AudioFormat::Unspecified:
            ALOGD("format = %s", "Decided by oboe");
            break;
        default:
        case oboe::AudioFormat::Invalid:
            ALOGD("format = %s", "Invalid");
            break;
    }

    switch (stream->getSampleRateConversionQuality()) {
        case oboe::SampleRateConversionQuality::None: ALOGD("sample rate conversion quality: %s", "None"); break;
        case oboe::SampleRateConversionQuality::Fastest: ALOGD("sample rate conversion quality: %s", "Fastest"); break;
        case oboe::SampleRateConversionQuality::Low: ALOGD("sample rate conversion quality: %s", "Low"); break;
        case oboe::SampleRateConversionQuality::Medium: ALOGD("sample rate conversion quality: %s", "Medium"); break;
        case oboe::SampleRateConversionQuality::High: ALOGD("sample rate conversion quality: %s", "High"); break;
        case oboe::SampleRateConversionQuality::Best: ALOGD("sample rate conversion quality: %s", "Best"); break;
    }

    switch (stream->getPerformanceMode()) {
        case oboe::PerformanceMode::None: ALOGD("performance mode = %s", "None"); break;
        case oboe::PerformanceMode::PowerSaving: ALOGD("performance mode = %s", "PowerSaving"); break;
        case oboe::PerformanceMode::LowLatency: ALOGD("performance mode = %s", "LowLatency"); break;
    }

    ALOGD("buffer capacity = %d frames", stream->getBufferCapacityInFrames());
    ALOGD("buffer size = %d frames", stream->getBufferSizeInFrames());
    ALOGD("frames per burst = %d", stream->getFramesPerBurst());
    ALOGD("frames per callback = %d", stream->getFramesPerCallback());
    if (stream->isXRunCountSupported()) ALOGD("--> xruns = %d", stream->getXRunCount().value());
}

struct OneWayStreamParams {
  int sampleRate;
  int numChannels; 
  int hostBlockSize; 
};

class OboeStream {
public:
  virtual ~OboeStream() = default;

  virtual oboe::Result open() = 0;
  virtual oboe::Result close() = 0;
  virtual oboe::Result start() = 0;
  virtual oboe::Result pause() = 0;
  virtual oboe::Result stop() = 0;
  virtual bool isRunning() const = 0;
};

class OneWayStream : public OboeStream, public oboe::AudioStreamCallback {
private:
  oboe::AudioStreamBuilder streamBuilder;
  oboe::ManagedStream stream;

  void* pdContext = nullptr;
  oboe_process_t pdSyncProc = nullptr;

protected:
  virtual oboe::Direction getDirection() const = 0;
  virtual void syncAudioDataWithPd(
      void* pdContext, oboe_process_t pdSyncProc,
      oboe::AudioStream *oboeStream, void *audioData, int32_t numFrames
  ) = 0;

  OneWayStream(OneWayStreamParams params, oboe::AudioStreamCallback* callback = nullptr) { 
    streamBuilder
      .setSampleRate(params.sampleRate)
      // Why hard-coding to OpenSL avoiding AAudio?
      // AAudio stream does not seamlessly continue playback
      // when connecting or disconnecting headphones or bluetooth speakers.
      // Ref: https://github.com/google/oboe/issues/823
      ->setAudioApi(oboe::AudioApi::OpenSLES)
      ->setChannelCount(params.numChannels)
      ->setCallback(callback)
      ->setFramesPerCallback(BLOCKS_PER_CALLBACK * params.hostBlockSize)
      ->setFormat(oboe::AudioFormat::Float)
      ->setFormatConversionAllowed(true)
      ->setSharingMode(oboe::SharingMode::Exclusive)
      ->setPerformanceMode(oboe::PerformanceMode::LowLatency);
  }

  OneWayStream(OneWayStreamParams params, void* pdContext, oboe_process_t pdSyncProc) : 
    OneWayStream(params, this) {
      this->pdContext = pdContext; 
      this->pdSyncProc = pdSyncProc; 
  }

public:
  virtual ~OneWayStream() = default;

  oboe::AudioStream* getStream() {
    return stream.get();
  }

  oboe::Result open() override {
    ALOGD("OneWayStream: Open ------");

    auto result = streamBuilder
                    .setDirection(getDirection())
                    ->openManagedStream(stream);

    logStreamConfig(stream);
    return result;
  }

  oboe::Result close() override {
    ALOGD("OneWayStream: close ------");
    return stream->close();
  }

  oboe::Result start() override {
    ALOGD("OneWayStream: start ------");

    logStreamConfig(stream);
    return stream->start();
  }

  oboe::Result pause() override {
    ALOGD("OneWayStream: pause ------");

    logStreamConfig(stream);
    return stream->pause();
  }

  oboe::Result stop() override {
    ALOGD("OneWayStream: stop ------");

    logStreamConfig(stream);
    return stream->stop();
  }

  bool isRunning() const override {
    oboe::StreamState state = stream->getState();
    return state == oboe::StreamState::Started || state == oboe::StreamState::Starting; 
  }

  oboe::DataCallbackResult 
  onAudioReady(AudioStream *oboeStream, void *audioData, int32_t numFrames) override {
    if (pdSyncProc != nullptr) {
      syncAudioDataWithPd(pdContext, pdSyncProc, oboeStream, audioData, numFrames);
      return oboe::DataCallbackResult::Continue;
    } else {
      return oboe::DataCallbackResult::Stop;
    }
  }

};

class InputStream : public OneWayStream {
private:
  oboe::Direction getDirection() const override {
      return oboe::Direction::Input;
  }

public:
  InputStream(OneWayStreamParams params) : OneWayStream(params) {}
  InputStream(OneWayStreamParams params, void* pdContext, oboe_process_t pdSyncProc) :
    OneWayStream(params, pdContext, pdSyncProc) {}

  virtual ~InputStream() = default;

  void syncAudioDataWithPd(
      void* pdContext, oboe_process_t pdSyncProc,
      AudioStream *oboeStream, void *audioData, int32_t numFrames
  ) override {
    pdSyncProc(pdContext, 
        oboeStream->getSampleRate(),
        numFrames,
        oboeStream->getChannelCount(),
        static_cast<const float *>(audioData),
        0, NULL
    );
  }

  oboe::Result pause() override {
    // Why calling stop() instead of pause()?
    // In AAudio, pause is not implemented for input streams,
    // it is recommended to use stop instead of pause for input stream.
    // To be uniform, oboe also enforces this behaviour even for opensl streams.
    //
    // What's the difference between pause and stop?
    // As per AAudio doc,
    // stop -> "The stream will stop after all of the data currently buffered has been played."
    // pause -> "Pausing a stream will freeze the data flow but not flush any buffers."
    return stop();
  }

};

class OutputStream : public OneWayStream {
private:
  oboe::Direction getDirection() const override {
      return oboe::Direction::Output;
  }

public:
  OutputStream(OneWayStreamParams params, oboe::AudioStreamCallback* callback) :
    OneWayStream(params, callback) {}
  OutputStream(OneWayStreamParams params, void* pdContext, oboe_process_t pdSyncProc) :
    OneWayStream(params, pdContext, pdSyncProc) {}

  virtual ~OutputStream() = default;

  void syncAudioDataWithPd(
      void* pdContext, oboe_process_t pdSyncProc,
      AudioStream *oboeStream, void *audioData, int32_t numFrames
  ) override {
    pdSyncProc(pdContext, 
        oboeStream->getSampleRate(),
        numFrames,
        0, NULL,
        oboeStream->getChannelCount(),
        static_cast<float *>(audioData)
    );
  }

};

class DuplexStream : public OboeStream, public FullDuplexStream {
private:
  std::unique_ptr<InputStream> inputStream;
  std::unique_ptr<OutputStream> outputStream;

  void* pdContext = nullptr;
  oboe_process_t pdSyncProc = nullptr;

public:
  DuplexStream(
      int sampleRate,
      int hostBlockSize, 
      int numInputChannels, 
      int numOutputChannels, 
      void* pdContext, oboe_process_t pdSyncProc) : 
    pdContext(pdContext),
    pdSyncProc(pdSyncProc) {

      auto outputParams = OneWayStreamParams {
        .sampleRate = sampleRate,
        .numChannels = numOutputChannels,
        .hostBlockSize = hostBlockSize
      };
      outputStream = std::make_unique<OutputStream>(outputParams, this);

      auto inputParams = OneWayStreamParams {
        .sampleRate = sampleRate,
        .numChannels = numInputChannels,
        .hostBlockSize = hostBlockSize
      };
      inputStream = std::make_unique<InputStream>(inputParams); // Uses oboe default callback
      setInputStream(inputStream->getStream());
  }

  virtual ~DuplexStream() = default;

  oboe::Result open() override {
    auto result = inputStream->open();
    if (result != oboe::Result::OK) { return result; }
    setInputStream(inputStream->getStream());

    result = outputStream->open();
    if (result != oboe::Result::OK) { return result; }
    setOutputStream(outputStream->getStream());

    return result;
  }

  oboe::Result close() override {
    auto inResult = inputStream->close();
    auto outResult = outputStream->close();

    return (inResult != oboe::Result::OK) ? inResult : outResult;
  }

  oboe::Result start() override {
    return FullDuplexStream::start();
  }

  oboe::Result stop() override {
    return FullDuplexStream::stop();
  }

  oboe::Result pause() override {
    auto inResult = inputStream->pause();
    auto outResult = outputStream->pause();

    return (inResult != oboe::Result::OK) ? inResult : outResult;
  }

  bool isRunning() const override {
    return inputStream->isRunning() && outputStream->isRunning();
  }

  oboe::DataCallbackResult
  onBothStreamsReady(const void *inputData, int numInputFrames, 
                     void *outputData, int numOutputFrames) override {
    auto in = inputStream->getStream();
    auto out = outputStream->getStream();
    pdSyncProc(pdContext, 
        out->getSampleRate(),
        numInputFrames, // TODO: Is this correct, or should we send number of pd blocks?
        in->getChannelCount(), static_cast<const float *>(inputData),
        out->getChannelCount(), static_cast<float *>(outputData)
    );
    return oboe::DataCallbackResult::Continue;
  }
};

OBOE_STREAM *oboe_open(
    int sampleRate, int inChans, int outChans, int hostBlockSize,
    oboe_process_t proc, void *context) {
    ALOGD("open ----");

  if (!proc) {
    return NULL;
  }
  if (inChans == 0 && outChans == 0) {
    return NULL;
  }

  OboeStream *oboeStream = nullptr;

  if (inChans > 0 && outChans > 0) {
    oboeStream = new DuplexStream(sampleRate, hostBlockSize, 
                         inChans, outChans, 
                         context, proc);

  } else if (inChans > 0) {
    auto inputParams = OneWayStreamParams {
      .sampleRate = sampleRate,
      .numChannels = inChans,
      .hostBlockSize = hostBlockSize
    };
    oboeStream = new InputStream(inputParams, context, proc);

  } else if (outChans > 0) {
    auto outputParams = OneWayStreamParams {
      .sampleRate = sampleRate,
      .numChannels = outChans,
      .hostBlockSize = hostBlockSize
    };
    oboeStream = new OutputStream(outputParams, context, proc);

  }

  auto result = oboeStream->open();
  if (result == oboe::Result::OK) {
    ALOGI("Created OBOE_STREAM(%d, %d, %d, %d)",
         sampleRate, inChans, outChans, hostBlockSize);
    return oboeStream;

  } else {
    ALOGE("Unable to create OBOE_STREAM(%d, %d, %d, %d). Result: %d",
         sampleRate, inChans, outChans, hostBlockSize, result);
    return NULL;

  }
}

void oboe_close(OBOE_STREAM *p) {
    ALOGD("close ----");
  auto oboeStream = static_cast<OboeStream*>(p);

  auto result = oboeStream->close();
  if (result != oboe::Result::OK) {
    ALOGE("Unable to close OBOE_STREAM. Result: %d", result);
  }

  delete oboeStream;
}

int oboe_is_running(OBOE_STREAM *p) {
  auto oboeStream = static_cast<OboeStream*>(p);

  return oboeStream->isRunning();
}

int oboe_start(OBOE_STREAM *p) {
  ALOGD("start ----");
  auto oboeStream = static_cast<OboeStream*>(p);

  if (oboeStream->isRunning()) {
    return 0;  // Already running.
  }

  auto result = oboeStream->start();
  if (result != oboe::Result::OK) {
    oboe_pause(p);
    return -1;
  }

  return 0;
}

void oboe_pause(OBOE_STREAM *p) {
  ALOGD("pause ----");
  auto oboeStream = static_cast<OboeStream*>(p);

  if (!oboeStream->isRunning()) {
    return;
  }

  auto result = oboeStream->stop();
  if (result != oboe::Result::OK) {
    ALOGE("Unable to pause OBOE_STREAM. Result: %d", result);
  }
}
