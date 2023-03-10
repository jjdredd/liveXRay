#include "stdafx.h"

#include "SoundRender_CoreA.h"

XRSOUND_API xr_token* snd_devices_token = nullptr;
XRSOUND_API u32 snd_device_id = u32(-1);

void ISoundManager::_create_devices_list()
{
    SoundRenderA = xr_new<CSoundRender_CoreA>();
    SoundRender = SoundRenderA;
    GEnv.Sound = SoundRender;
    SoundRender->bPresent = strstr(Core.Params, "-nosound") == nullptr;
    if (SoundRender->bPresent)
        GEnv.Sound->_initialize_devices_list();
}

void ISoundManager::_create()
{
    if (SoundRender->bPresent)
        GEnv.Sound->_initialize();
}

void ISoundManager::_destroy()
{
    GEnv.Sound->_clear();
    xr_delete(SoundRender);
    GEnv.Sound = nullptr;
}
