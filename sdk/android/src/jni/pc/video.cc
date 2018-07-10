/*
 *  Copyright 2017 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "sdk/android/src/jni/pc/video.h"

#include <jni.h>
#include <memory>

#include "api/video_codecs/builtin_video_decoder_factory.h"
#include "api/video_codecs/builtin_video_encoder_factory.h"
#include "api/video_codecs/video_decoder_factory.h"
#include "api/video_codecs/video_encoder_factory.h"
#include "api/videosourceproxy.h"
#include "media/engine/convert_legacy_video_factory.h"
#include "rtc_base/logging.h"
#include "rtc_base/ptr_util.h"
#include "sdk/android/generated_video_jni/jni/EglBase14_jni.h"
#include "sdk/android/src/jni/androidmediadecoder_jni.h"
#include "sdk/android/src/jni/androidmediaencoder_jni.h"
#include "sdk/android/src/jni/androidvideotracksource.h"
#include "sdk/android/src/jni/videodecoderfactorywrapper.h"
#include "sdk/android/src/jni/videoencoderfactorywrapper.h"

namespace webrtc {
namespace jni {

VideoEncoderFactory* CreateVideoEncoderFactory(
    JNIEnv* jni,
    const JavaRef<jobject>& j_encoder_factory) {
  return new VideoEncoderFactoryWrapper(jni, j_encoder_factory);
}

VideoDecoderFactory* CreateVideoDecoderFactory(
    JNIEnv* jni,
    const JavaRef<jobject>& j_decoder_factory) {
  return new VideoDecoderFactoryWrapper(jni, j_decoder_factory);
}

void SetEglContext(JNIEnv* env,
                   VideoEncoderFactory* encoder_factory,
                   const JavaRef<jobject>& egl_context) {
  if (encoder_factory) {
    MediaCodecVideoEncoderFactory* media_codec_factory =
        static_cast<MediaCodecVideoEncoderFactory*>(encoder_factory);
    if (media_codec_factory && Java_Context_isEgl14Context(env, egl_context)) {
      RTC_LOG(LS_INFO) << "Set EGL context for HW encoding.";
      media_codec_factory->SetEGLContext(env, egl_context.obj());
    }
  }
}

void SetEglContext(JNIEnv* env,
                   VideoDecoderFactory* decoder_factory,
                   const JavaRef<jobject>& egl_context) {
  if (decoder_factory) {
    MediaCodecVideoDecoderFactory* media_codec_factory =
        static_cast<MediaCodecVideoDecoderFactory*>(decoder_factory);
    if (media_codec_factory) {
      RTC_LOG(LS_INFO) << "Set EGL context for HW decoding.";
      media_codec_factory->SetEGLContext(env, egl_context.obj());
    }
  }
}

void* CreateVideoSource(JNIEnv* env,
                        rtc::Thread* signaling_thread,
                        rtc::Thread* worker_thread,
                        jboolean is_screencast) {
  rtc::scoped_refptr<AndroidVideoTrackSource> source(
      new rtc::RefCountedObject<AndroidVideoTrackSource>(signaling_thread, env,
                                                         is_screencast));
  return VideoTrackSourceProxy::Create(signaling_thread, worker_thread, source)
      .release();
}

std::unique_ptr<VideoEncoderFactory> CreateLegacyVideoEncoderFactory() {
  return rtc::MakeUnique<MediaCodecVideoEncoderFactory>();
}

std::unique_ptr<VideoDecoderFactory> CreateLegacyVideoDecoderFactory() {
  return rtc::MakeUnique<MediaCodecVideoDecoderFactory>();
}

std::unique_ptr<VideoEncoderFactory> WrapLegacyVideoEncoderFactory(
    std::unique_ptr<VideoEncoderFactory> legacy_encoder_factory) {
  return legacy_encoder_factory ? std::unique_ptr<VideoEncoderFactory>(
                                      cricket::ConvertVideoEncoderFactory(
                                          std::move(legacy_encoder_factory)))
                                : CreateBuiltinVideoEncoderFactory();
}

std::unique_ptr<VideoDecoderFactory> WrapLegacyVideoDecoderFactory(
    std::unique_ptr<VideoDecoderFactory> legacy_decoder_factory) {
  return legacy_decoder_factory ? std::unique_ptr<VideoDecoderFactory>(
                                      cricket::ConvertVideoDecoderFactory(
                                          std::move(legacy_decoder_factory)))
                                : CreateBuiltinVideoDecoderFactory();
}

}  // namespace jni
}  // namespace webrtc
