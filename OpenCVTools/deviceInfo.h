#pragma once
#include <vector>
#include <string>

// 设备类型枚举
enum DeviceType {
	DEVICE_TYPE_VIDEO = 0,
	DEVICE_TYPE_AUDIO,
	DEVICE_TYPE_UNKNOWN
};

// 视频格式支持
struct VideoFormat {
	int width;
	int height;
	int fps;
	int pixel_format;  // AV_PIX_FMT_*
	std::string format_name;
};

// 音频格式支持
struct AudioFormat {
	int sample_rate;
	int channels;
	int sample_format; // AV_SAMPLE_FMT_*
	std::string format_name;
};

// 设备能力信息
struct DeviceInfo {
	std::vector<VideoFormat> video_formats;
	std::vector<AudioFormat> audio_formats;
	bool can_capture;    // 是否支持采集
	bool can_playback;   // 是否支持播放
	int min_width;
	int max_width;
	int min_height;
	int max_height;
	int min_fps;
	int max_fps;
};

// 增强的设备信息结构
struct HDevice {
	char name[256];
	char id[256];
	DeviceType type;
	DeviceInfo capabilities;
	
	HDevice() {
		memset(name, 0, sizeof(name));
		memset(id, 0, sizeof(id));
		type = DEVICE_TYPE_UNKNOWN;
	}
};

// 设备管理类
class DeviceManager {
public:
	static std::vector<HDevice> getVideoDevices();
	static std::vector<HDevice> getAudioDevices();
	static bool testDevice(const std::string& device_name, DeviceType type);
	static DeviceInfo getDeviceCapabilities(const std::string& device_name, DeviceType type);
	static bool isDeviceSupported(const std::string& device_name, const VideoFormat& format);
	static bool isDeviceSupported(const std::string& device_name, const AudioFormat& format);
	
private:
	static void enumerateDeviceFormats(HDevice& device);
	static bool testVideoDevice(const std::string& device_name);
	static bool testAudioDevice(const std::string& device_name);
};

// 兼容性函数（保持向后兼容）
std::vector<HDevice> getVideoDevices();
std::vector<HDevice> getAudioDevices();

