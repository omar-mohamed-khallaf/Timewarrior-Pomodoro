#include <fstream>
#include <memory>
#include <vector>
#include <vorbis/vorbisfile.h>

#include "utils.h"
#include "sound/platform/desktop/OpenAlAudioPlayer.h"

#define alCall(function, ...) alCallImpl(__FILE__, __LINE__, function, __VA_ARGS__)
#define alcCall(function, ...) alcCallImpl(__FILE__, __LINE__, function, __VA_ARGS__)

static bool checkAlErrors(const std::string &filename, const std::uint_fast32_t line) {
    ALenum error = alGetError();
    if (error != AL_NO_ERROR) {
        std::cerr << "***ERROR*** (" << filename << ": " << line << ")\n";
        switch (error) {
            case AL_INVALID_NAME:
                std::cerr << "AL_INVALID_NAME: a bad name (ID) was passed to an OpenAL function";
                break;
            case AL_INVALID_ENUM:
                std::cerr << "AL_INVALID_ENUM: an invalid enum value was passed to an OpenAL function";
                break;
            case AL_INVALID_VALUE:
                std::cerr << "AL_INVALID_VALUE: an invalid value was passed to an OpenAL function";
                break;
            case AL_INVALID_OPERATION:
                std::cerr << "AL_INVALID_OPERATION: the requested operation is not valid";
                break;
            case AL_OUT_OF_MEMORY:
                std::cerr << "AL_OUT_OF_MEMORY: the requested operation resulted in OpenAL running out of memory";
                break;
            default:
                std::cerr << "UNKNOWN AL ERROR: " << error;
        }
        std::cerr << std::endl;
        return false;
    }
    return true;
}

bool checkAlcErrors(const std::string &filename, const std::uint_fast32_t line, ALCdevice *device) {
    ALCenum error = alcGetError(device);
    if (error != ALC_NO_ERROR) {
        std::cerr << "***ERROR*** (" << filename << ": " << line << ")\n";
        switch (error) {
            case ALC_INVALID_VALUE:
                std::cerr << "ALC_INVALID_VALUE: an invalid value was passed to an OpenAL function";
                break;
            case ALC_INVALID_DEVICE:
                std::cerr << "ALC_INVALID_DEVICE: a bad device was passed to an OpenAL function";
                break;
            case ALC_INVALID_CONTEXT:
                std::cerr << "ALC_INVALID_CONTEXT: a bad context was passed to an OpenAL function";
                break;
            case ALC_INVALID_ENUM:
                std::cerr << "ALC_INVALID_ENUM: an unknown enum value was passed to an OpenAL function";
                break;
            case ALC_OUT_OF_MEMORY:
                std::cerr << "ALC_OUT_OF_MEMORY: an unknown enum value was passed to an OpenAL function";
                break;
            default:
                std::cerr << "UNKNOWN ALC ERROR: " << error;
        }
        std::cerr << std::endl;
        return false;
    }
    return true;
}

template<typename AlFunction, typename ...Params>
auto alCallImpl(const char *filename, std::uint_fast32_t line, AlFunction function,
                Params... params) -> typename std::enable_if_t<!std::is_same_v<void, decltype(function(
        params...))>, decltype(function(params...))> {
    auto ret = function(std::forward<Params>(params)...);
    checkAlErrors(ret);
    return ret;
}

template<typename AlFunction, typename ...Params>
auto alCallImpl(const char *filename, std::uint_fast32_t line, AlFunction function,
                Params... params) -> std::enable_if_t<std::is_same_v<void, decltype(function(params...))>, bool> {
    function(std::forward<Params>(params)...);
    return checkAlErrors(filename, line);
}

template<typename AlcFunction, typename ReturnType, typename... Params>
auto alcCallImpl(const char *filename, const std::uint_fast32_t line, AlcFunction function, ReturnType &ret,
                 ALCdevice *device,
                 Params... params) -> typename std::enable_if_t<!std::is_same_v<void, decltype(function(
        params...))>, bool> {
    ret = function(std::forward<Params>(params)...);
    return checkAlcErrors(filename, line, device);
}

template<typename AlcFunction, typename... Params>
auto alcCallImpl(const char *filename, const std::uint_fast32_t line, AlcFunction function, ALCdevice *device,
                 Params... params) -> typename std::enable_if_t<std::is_same_v<void, decltype(function(
        params...))>, bool> {
    function(std::forward<Params>(params)...);
    return checkAlcErrors(filename, line, device);
}

OpenAlAudioPlayer::OpenAlAudioPlayer() : openALDevice_{alcOpenDevice(nullptr)} {
    if (openALDevice_ == nullptr) throw std::runtime_error("Failed to initialize AL Device");

    alcCall(alcCreateContext, openALContext_, openALDevice_, openALDevice_, nullptr);
    if (openALContext_ == nullptr) throw std::runtime_error("Failed to initialize AL Context");

    if (!alcCall(alcMakeContextCurrent, contextCurrent_, openALDevice_, openALContext_) || contextCurrent_ != ALC_TRUE)
        throw std::runtime_error("Could not make audio context current");
}

OpenAlAudioPlayer::~OpenAlAudioPlayer() {
    alcCall(alcMakeContextCurrent, contextCurrent_, openALDevice_, nullptr);
    alcCall(alcDestroyContext, openALDevice_, openALContext_);
    ALCboolean closed;
    alcCall(alcCloseDevice, closed, openALDevice_, openALDevice_);
}

void OpenAlAudioPlayer::load(const std::string &audioFile) {
    auto extension = audioFile.substr(audioFile.find_last_of('.'));
    auto dataSource = std::unique_ptr<FILE, decltype(&fclose)>(fopen(audioFile.c_str(), "rb"), fclose);
    if (dataSource == nullptr) throw std::runtime_error("Failed to open file: " + audioFile);

    ALuint alSource;
    alCall(alGenSources, 1, &alSource);
    alCall(alSourcef, alSource, AL_PITCH, 1.0f);
    alCall(alSourcef, alSource, AL_GAIN, 1.0f);
    alCall(alSource3f, alSource, AL_POSITION, 0.0f, 0.0f, 0.0f);
    alCall(alSource3f, alSource, AL_VELOCITY, 0.0f, 0.0f, 0.0f);
    alCall(alSourcei, alSource, AL_LOOPING, AL_FALSE);
    ALuint alBuffer;
    alCall(alGenBuffers, 1, &alBuffer);

    if (extension == ".ogg") {
        OggVorbis_File vorbisFile;
        ov_open_callbacks(dataSource.get(), &vorbisFile, nullptr, -1, OV_CALLBACKS_DEFAULT);
        auto vorbisInfo = std::unique_ptr<vorbis_info>(ov_info(&vorbisFile, -1));
        std::vector<char> decodedSound;
        char buffer[4096];
        for (ssize_t size; (size = ov_read(&vorbisFile, buffer, 4096, 0, 2, 1, nullptr)) != 0;) {
            if (size < 0) throw std::runtime_error("Failed to decode ogg file");
            decodedSound.insert(decodedSound.end(), buffer, buffer + size);
        }
        alCall(alBufferData, alBuffer, utils::getAlAudioFormat(vorbisInfo->channels, 16), decodedSound.data(),
               decodedSound.size(), vorbisInfo->rate);
    } else if (extension == ".wav") {
        unsigned int channel, sampleRate, bps, size;
        auto buffer = utils::WavReader::loadWAV(audioFile, channel, sampleRate, bps, size);
        alCall(alBufferData, alBuffer, utils::getAlAudioFormat(channel, bps), buffer.get(), size, sampleRate);
    } else {
        throw std::runtime_error("Unsupported audio format (" + extension + ")");
    }

    alCall(alSourcei, alSource, AL_BUFFER, alBuffer);
    audio_[audioFile] = alSource;
}

void OpenAlAudioPlayer::play(const std::string &audioFile) {
    alCall(alSourcePlay, audio_.at(audioFile));
}
