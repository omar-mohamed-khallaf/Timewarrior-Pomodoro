#include <cassert>
#include "sound/platform/android/OpenSlAudioPlayer.h"


OpenSlAudioPlayer::OpenSlAudioPlayer() {
    SLresult result = slCreateEngine(&slEngineObj_, 0, nullptr, 0, nullptr, nullptr);
    assert(SL_RESULT_SUCCESS == result);

    result = (*slEngineObj_)->Realize(slEngineObj_, SL_BOOLEAN_FALSE);
    assert(SL_RESULT_SUCCESS == result);

    result = (*slEngineObj_)->GetInterface(slEngineObj_, SL_IID_ENGINE, &slEngineItf_);
    assert(SL_RESULT_SUCCESS == result);

    result = (*slEngineItf_)->CreateOutputMix(slEngineItf_, &slOutputMixObj_, 0, nullptr, nullptr);
    assert(SL_RESULT_SUCCESS == result);

    result = (*slOutputMixObj_)->Realize(slOutputMixObj_, SL_BOOLEAN_FALSE);
    assert(SL_RESULT_SUCCESS == result);
}

OpenSlAudioPlayer::~OpenSlAudioPlayer() {
    (*slAudioPlayerObj_)->Destroy(slAudioPlayerObj_);
    (*slOutputMixObj_)->Destroy(slOutputMixObj_);
    (*slEngineObj_)->Destroy(slEngineObj_);
}

void OpenSlAudioPlayer::load(const std::string &audioFile) {
    SLresult result;
    // configure audio source
    SLDataLocator_URI slDataLocatorUri = {SL_DATALOCATOR_URI, (SLchar *) audioFile.c_str()};
    SLDataFormat_MIME slDataFormatMime = {SL_DATAFORMAT_MIME, nullptr, SL_CONTAINERTYPE_UNSPECIFIED};
    SLDataSource slDataSource = {&slDataLocatorUri, &slDataFormatMime};

    // configure audio sink
    SLDataLocator_OutputMix slDataLocatorOutputMix = {SL_DATALOCATOR_OUTPUTMIX, slOutputMixObj_};
    SLDataSink audioSnk = {&slDataLocatorOutputMix, nullptr};

    // create audio player
    std::size_t reqItfSize = 1;
    SLInterfaceID ids[2] = {SL_IID_SEEK, nullptr};
    SLboolean req[2] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};
#ifdef __ANDROID__
    ids[reqItfSize++] = SL_IID_ANDROIDCONFIGURATION;
#endif
    (*slEngineItf_)->CreateAudioPlayer(slEngineItf_, &slAudioPlayerObj_, &slDataSource, &audioSnk, reqItfSize, ids,
                                       req);
#ifdef __ANDROID__
    SLAndroidConfigurationItf androidConfig;
    result = (*slAudioPlayerObj_)->GetInterface(slAudioPlayerObj_, SL_IID_ANDROIDCONFIGURATION, &androidConfig);
    assert(SL_RESULT_SUCCESS == result);
    result = (*androidConfig)->SetConfiguration(androidConfig, SL_ANDROID_KEY_STREAM_TYPE, &this->androidStreamType, sizeof(SLint32));
    assert(SL_RESULT_SUCCESS == result);
#endif

    result = (*slAudioPlayerObj_)->Realize(slAudioPlayerObj_, SL_BOOLEAN_FALSE);
    assert(SL_RESULT_SUCCESS == result);

    SLSeekItf audioPlayerSeekItf;
    result = (*slAudioPlayerObj_)->GetInterface(slAudioPlayerObj_, SL_IID_SEEK, &audioPlayerSeekItf);
    assert(SL_RESULT_SUCCESS == result);
    result = (*audioPlayerSeekItf)->SetLoop(audioPlayerSeekItf, (SLboolean) false, 0, SL_TIME_UNKNOWN);
    assert(SL_RESULT_SUCCESS == result);

    SLPlayItf audioPlayerPlayItf;
    result = (*slAudioPlayerObj_)->GetInterface(slAudioPlayerObj_, SL_IID_PLAY, &audioPlayerPlayItf);
    assert(SL_RESULT_SUCCESS == result);
    audio_[audioFile] = audioPlayerPlayItf;
}

void OpenSlAudioPlayer::play(const std::string &audioFile) noexcept(true) {
    (*audio_.at(audioFile))->SetPlayState(audio_[audioFile], SL_PLAYSTATE_STOPPED);
    (*audio_.at(audioFile))->SetPlayState(audio_[audioFile], SL_PLAYSTATE_PLAYING);
}
