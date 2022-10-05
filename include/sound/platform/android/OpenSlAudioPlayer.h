#pragma once

#include <string>
#include <SLES/OpenSLES.h>
#include <unordered_map>

#ifdef __ANDROID__
#include <SLES/OpenSLES_Android.h>
#endif

/**
 * @remark https://github.com/android/ndk-samples/blob/master/native-audio/app/src/main/cpp/native-audio-jni.c
 */
class OpenSlAudioPlayer {
public:
    OpenSlAudioPlayer();


    ~OpenSlAudioPlayer();

    /**
     * Loads an audio file
     */
    void load(const std::string &);

    /**
     * Plays an audio file
     */
    void play(const std::string &);

private:
    SLEngineItf slEngineItf_{nullptr};
    SLObjectItf slEngineObj_{nullptr};
    SLObjectItf slOutputMixObj_{nullptr};
    SLObjectItf slAudioPlayerObj_{nullptr};
#ifdef __ANDROID__
    SLint32 androidStreamType{SL_ANDROID_STREAM_NOTIFICATION};
#endif
    std::unordered_map<std::string, SLPlayItf> audio_;
};
