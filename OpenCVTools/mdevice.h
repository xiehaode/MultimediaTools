#ifndef MDEVICE_H
#define MDEVICE_H

#include "pch.h"


struct HDevice {
	char name[256];
};

std::vector<HDevice> getVideoDevices();
std::vector<HDevice> getAudioDevices();

#endif // MDEVICE_H

