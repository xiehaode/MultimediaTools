
#include "pch.h"
#include "deviceInfo.h"
#include "OpenCVFFMpegTools.h"

#include <cstring>
#include <windows.h>
#include <dshow.h>

#ifdef __cplusplus
extern "C" {
#endif
#include <libavformat/avformat.h>
#include <libavdevice/avdevice.h>
#ifdef __cplusplus
}
#endif

#pragma comment(lib, "strmiids.lib")

static std::vector<HDevice> getDevicesList(REFGUID category) {
	std::vector<HDevice> ret;
	HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	if (hr < 0) {
		printf("com init err hr: %d",hr);
	}
	hr = S_OK;
	if (SUCCEEDED(hr)) {
		ICreateDevEnum *pDevEnum = NULL;
		HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL,
			CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pDevEnum));
		if (SUCCEEDED(hr) && pDevEnum) {
			IEnumMoniker *pEnum = NULL;
			hr = pDevEnum->CreateClassEnumerator(category, &pEnum, 0);
			if (hr == S_FALSE || pEnum == NULL) {
				pDevEnum->Release();
				return ret;
			}

			IMoniker *pMoniker = NULL;
			while (pEnum->Next(1, &pMoniker, NULL) == S_OK && pMoniker) {
				IPropertyBag *pPropBag = NULL;
				HRESULT hr = pMoniker->BindToStorage(0, 0, IID_PPV_ARGS(&pPropBag));
				if (FAILED(hr) || pPropBag == NULL) {
					pMoniker->Release();
					continue;
				}

				VARIANT var;
				VariantInit(&var);

				// Get description or friendly name.
				hr = pPropBag->Read(L"Description", &var, 0);
				if (FAILED(hr)) {
					hr = pPropBag->Read(L"FriendlyName", &var, 0);
				}
				if (SUCCEEDED(hr)) {
					HDevice dev;
					sprintf(dev.name, "%S", var.bstrVal);
					ret.push_back(dev);
					//printf("%S\n", var.bstrVal);
					VariantClear(&var);
				}

				//                hr = pPropBag->Write(L"FriendlyName", &var);

				//                // WaveInID applies only to audio capture devices.
				//                hr = pPropBag->Read(L"WaveInID", &var, 0);
				//                if (SUCCEEDED(hr)) {
				//                    printf("WaveIn ID: %d\n", var.lVal);
				//                    VariantClear(&var);
				//                }

				//                hr = pPropBag->Read(L"DevicePath", &var, 0);
				//                if (SUCCEEDED(hr)) {
				//                    // The device path is not intended for display.
				//                    printf("Device path: %S\n", var.bstrVal);
				//                    VariantClear(&var);
				//                }

				pPropBag->Release();
				pMoniker->Release();
			}

			pEnum->Release();
			pDevEnum->Release();
		}

		//CoUninitialize();
	}

	return ret;
}

std::vector<HDevice> getVideoDevices() {
	return getDevicesList(CLSID_VideoInputDeviceCategory);
}


// 音频设备枚举实现
std::vector<HDevice> getAudioDevices() {
	return DeviceManager::getAudioDevices();
}

// DeviceManager实现
std::vector<HDevice> DeviceManager::getVideoDevices() {
	return getDevicesList(CLSID_VideoInputDeviceCategory);
}

std::vector<HDevice> DeviceManager::getAudioDevices() {
	return getDevicesList(CLSID_AudioInputDeviceCategory);
}

bool DeviceManager::testDevice(const std::string& device_name, DeviceType type) {
	if (type == DEVICE_TYPE_VIDEO) {
		return testVideoDevice(device_name);
	} else if (type == DEVICE_TYPE_AUDIO) {
		return testAudioDevice(device_name);
	}
	return false;
}

DeviceInfo DeviceManager::getDeviceCapabilities(const std::string& device_name, DeviceType type) {
	DeviceInfo caps;
	
	// 设置默认值
	caps.can_capture = true;
	caps.can_playback = false;
	caps.min_width = 320;
	caps.max_width = 1920;
	caps.min_height = 240;
	caps.max_height = 1080;
	caps.min_fps = 15;
	caps.max_fps = 60;
	
	// 尝试枚举支持的格式（这里简化处理）
	if (type == DEVICE_TYPE_VIDEO) {
		// 常见视频格式
		caps.video_formats = {
			{640, 480, 30, AV_PIX_FMT_YUV420P, "YUV420P"},
			{1280, 720, 30, AV_PIX_FMT_YUV420P, "720p YUV"},
			{1920, 1080, 30, AV_PIX_FMT_YUV420P, "1080p YUV"},
			{640, 480, 30, AV_PIX_FMT_BGR24, "BGR24"},
			{1280, 720, 30, AV_PIX_FMT_BGR24, "720p BGR"},
			{1920, 1080, 30, AV_PIX_FMT_BGR24, "1080p BGR"}
		};
	} else if (type == DEVICE_TYPE_AUDIO) {
		// 常见音频格式
		caps.audio_formats = {
			{44100, 2, AV_SAMPLE_FMT_S16, "44.1kHz Stereo"},
			{48000, 2, AV_SAMPLE_FMT_S16, "48kHz Stereo"},
			{44100, 1, AV_SAMPLE_FMT_S16, "44.1kHz Mono"},
			{48000, 1, AV_SAMPLE_FMT_S16, "48kHz Mono"}
		};
	}
	
	return caps;
}

bool DeviceManager::isDeviceSupported(const std::string& device_name, const VideoFormat& format) {
	DeviceInfo caps = getDeviceCapabilities(device_name, DEVICE_TYPE_VIDEO);
	
	// 检查分辨率和帧率是否在支持范围内
	if (format.width < caps.min_width || format.width > caps.max_width ||
		format.height < caps.min_height || format.height > caps.max_height ||
		format.fps < caps.min_fps || format.fps > caps.max_fps) {
		return false;
	}
	
	// 检查具体格式支持
	for (const auto& supported_format : caps.video_formats) {
		if (supported_format.width == format.width &&
			supported_format.height == format.height &&
			supported_format.pixel_format == format.pixel_format) {
			return true;
		}
	}
	
	return false;
}

bool DeviceManager::isDeviceSupported(const std::string& device_name, const AudioFormat& format) {
	DeviceInfo caps = getDeviceCapabilities(device_name, DEVICE_TYPE_AUDIO);
	
	// 检查具体格式支持
	for (const auto& supported_format : caps.audio_formats) {
		if (supported_format.sample_rate == format.sample_rate &&
			supported_format.channels == format.channels &&
			supported_format.sample_format == format.sample_format) {
			return true;
		}
	}
	
	return false;
}

void DeviceManager::enumerateDeviceFormats(HDevice& device) {
	device.capabilities = getDeviceCapabilities(std::string(device.name), device.type);
}

bool DeviceManager::testVideoDevice(const std::string& device_name) {
	AVFormatContext* fmt_ctx = nullptr;
	AVDictionary* opts = nullptr;
	
	// 设置测试参数
	av_dict_set(&opts, "framerate", "30", 0);
	av_dict_set(&opts, "video_size", "640x480", 0);
	
	std::string url = "video=" + device_name;
	int ret = avformat_open_input(&fmt_ctx, url.c_str(), 
		av_find_input_format("dshow"), &opts);
	
	if (ret >= 0) {
		avformat_close_input(&fmt_ctx);
		av_dict_free(&opts);
		return true;
	}
	
	av_dict_free(&opts);
	return false;
}

bool DeviceManager::testAudioDevice(const std::string& device_name) {
	AVFormatContext* fmt_ctx = nullptr;
	AVDictionary* opts = nullptr;
	
	std::string url = "audio=" + device_name;
	int ret = avformat_open_input(&fmt_ctx, url.c_str(), 
		av_find_input_format("dshow"), &opts);
	
	if (ret >= 0) {
		avformat_close_input(&fmt_ctx);
		av_dict_free(&opts);
		return true;
	}
	
	av_dict_free(&opts);
	return false;
}




// ---------------- C 接口实现 ----------------
OPENCVFFMPEGTOOLS_API int Device_GetVideoDeviceCount() {
	return static_cast<int>(DeviceManager::getVideoDevices().size());
}

OPENCVFFMPEGTOOLS_API bool Device_GetVideoDeviceName(int index, char* out_name, int out_name_len) {
	if (out_name == NULL || out_name_len <= 0 || index < 0) {
		return false;
	}
	std::vector<HDevice> devices = DeviceManager::getVideoDevices();
	if (index >= static_cast<int>(devices.size())) {
		return false;
	}
	// 安全拷贝（防止缓冲区溢出）
	strncpy(out_name, devices[index].name, out_name_len - 1);
	out_name[out_name_len - 1] = '\0'; // 确保字符串结束符
	return true;
}

OPENCVFFMPEGTOOLS_API int Device_GetAudioDeviceCount() {
	return static_cast<int>(DeviceManager::getAudioDevices().size());
}

OPENCVFFMPEGTOOLS_API bool Device_GetAudioDeviceName(int index, char* out_name, int out_name_len) {
	if (out_name == NULL || out_name_len <= 0 || index < 0) {
		return false;
	}
	std::vector<HDevice> devices = DeviceManager::getAudioDevices();
	if (index >= static_cast<int>(devices.size())) {
		return false;
	}
	strncpy(out_name, devices[index].name, out_name_len - 1);
	out_name[out_name_len - 1] = '\0';
	return true;
}

// 新增设备测试接口
OPENCVFFMPEGTOOLS_API bool Device_TestVideoDevice(const char* device_name) {
	if (!device_name) return false;
	return DeviceManager::testDevice(std::string(device_name), DEVICE_TYPE_VIDEO);
}

OPENCVFFMPEGTOOLS_API bool Device_TestAudioDevice(const char* device_name) {
	if (!device_name) return false;
	return DeviceManager::testDevice(std::string(device_name), DEVICE_TYPE_AUDIO);
}