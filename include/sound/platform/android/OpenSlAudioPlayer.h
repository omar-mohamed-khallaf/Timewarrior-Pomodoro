#pragma once

#include <string>
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <unordered_map>

/**
 * @remark https://github.com/android/ndk-samples/blob/master/native-audio/app/src/main/cpp/native-audio-jni.c
 */
class OpenSlAudioPlayer {
public:
    OpenSlAudioPlayer();


    ~OpenSlAudioPlayer();

    void load(const std::string &);

    void play(const std::string &);

private:
    SLObjectItf slEngineObjectItf_{nullptr};
    SLEngineItf slEngineItf_{nullptr};
    SLObjectItf slOutputMixObject_{nullptr};
    SLint32 androidStreamType{SL_ANDROID_STREAM_MEDIA};
    std::unordered_map<std::string, SLPlayItf> audio_;
};
