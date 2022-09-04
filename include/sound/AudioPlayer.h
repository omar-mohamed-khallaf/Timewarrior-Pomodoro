#pragma once

#ifdef __ANDROID__

#include "sound/platform/android/OpenSlAudioPlayer.h"

typedef OpenSlAudioPlayer AudioPlayer;
#else

#include "sound/platform/desktop/OpenAlAudioPlayer.h"

typedef OpenAlAudioPlayer AudioPlayer;
#endif