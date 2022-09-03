#include <cassert>
#include "sound/platform/android/OpenSlAudioPlayer.h"

OpenSlAudioPlayer::OpenSlAudioPlayer() {
    SLresult result = slCreateEngine(&slEngineObjectItf_, 0, nullptr, 0, nullptr, nullptr);
    assert(SL_RESULT_SUCCESS == result);

    result = (*slEngineObjectItf_)->Realize(slEngineObjectItf_, SL_BOOLEAN_FALSE);
    assert(SL_RESULT_SUCCESS == result);

    result = (*slEngineObjectItf_)->GetInterface(slEngineObjectItf_, SL_IID_ENGINE, &slEngineItf_);
    assert(SL_RESULT_SUCCESS == result);

    result = (*slEngineItf_)->CreateOutputMix(slEngineItf_, &slOutputMixObject_, 0, nullptr, nullptr);
    assert(SL_RESULT_SUCCESS == result);

    result = (*slOutputMixObject_)->Realize(slOutputMixObject_, SL_BOOLEAN_FALSE);
    assert(SL_RESULT_SUCCESS == result);


}

OpenSlAudioPlayer::~OpenSlAudioPlayer() {

}

void OpenSlAudioPlayer::load(const std::string &audioFile) {
    SLresult result;
    // configure audio source
    SLDataLocator_URI slDataLocatorUri = {SL_DATALOCATOR_URI, (SLchar *) audioFile.c_str()};
    SLDataFormat_MIME slDataFormatMime = {SL_DATAFORMAT_MIME, nullptr, SL_CONTAINERTYPE_UNSPECIFIED};
    SLDataSource slDataSource = {&slDataLocatorUri, &slDataFormatMime};

    // configure audio sink
    SLDataLocator_OutputMix slDataLocatorOutputMix = {SL_DATALOCATOR_OUTPUTMIX, slOutputMixObject_};
    SLDataSink audioSnk = {&slDataLocatorOutputMix, nullptr};

    // create audio player
    SLObjectItf uriPlayerObject;
    std::size_t reqItfSize = 1;
    SLInterfaceID ids[2] = {SL_IID_SEEK, nullptr};
    SLboolean req[2] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};
#ifdef __ANDROID__
    ids[reqItfSize++] = SL_IID_ANDROIDCONFIGURATION;
#endif
    (*slEngineItf_)->CreateAudioPlayer(slEngineItf_, &uriPlayerObject, &slDataSource, &audioSnk, reqItfSize, ids, req);
#ifdef __ANDROID__
    SLAndroidConfigurationItf androidConfig;
    result = (*uriPlayerObject)->GetInterface(uriPlayerObject, SL_IID_ANDROIDCONFIGURATION, &androidConfig);
    assert(SL_RESULT_SUCCESS == result);
    result = (*androidConfig)->SetConfiguration(androidConfig, SL_ANDROID_KEY_STREAM_TYPE, &this->androidStreamType, sizeof(SLint32));
    assert(SL_RESULT_SUCCESS == result);
#endif

    result = (*uriPlayerObject)->Realize(uriPlayerObject, SL_BOOLEAN_FALSE);
    // this will always succeed on Android, but we check result for portability to other platforms
    assert(SL_RESULT_SUCCESS == result);

    SLPlayItf uriPlayerItf;
    result = (*uriPlayerObject)->GetInterface(uriPlayerObject, SL_IID_PLAY, &uriPlayerItf);
    assert(SL_RESULT_SUCCESS == result);

    SLSeekItf uriPlayerSeekItf;
    result = (*uriPlayerObject)->GetInterface(uriPlayerObject, SL_IID_SEEK, &uriPlayerSeekItf);
    assert(SL_RESULT_SUCCESS == result);

    result = (*uriPlayerSeekItf)->SetLoop(uriPlayerSeekItf, (SLboolean) false, 0, SL_TIME_UNKNOWN);
    assert(SL_RESULT_SUCCESS == result);

    audio_[audioFile] = uriPlayerItf;
}

void OpenSlAudioPlayer::play(const std::string &audioFile) {
    (*audio_.at(audioFile))->SetPlayState(audio_[audioFile], SL_PLAYSTATE_PLAYING);
}
