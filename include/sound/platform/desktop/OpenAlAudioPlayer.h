#pragma once

#include <iostream>
#include <AL/al.h>
#include <AL/alc.h>
#include <unordered_map>

class OpenAlAudioPlayer {
public:
    OpenAlAudioPlayer();

    ~OpenAlAudioPlayer();

    void load(const std::string &);

    void play(const std::string &);

private:
    ALCdevice *openALDevice_ = nullptr;
    ALCcontext *openALContext_ = nullptr;
    ALCboolean contextCurrent_ = false;
    std::unordered_map<std::string, ALuint> audio_;
};