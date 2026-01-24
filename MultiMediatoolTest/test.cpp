#include "pch.h"
#include "../OpenCVTools/OpenCVFFMpegTools.h"

#include <string>

#include <Windows.h>

namespace {
std::string GetExeDir() {
	char path[MAX_PATH] = { 0 };
	DWORD len = GetModuleFileNameA(nullptr, path, MAX_PATH);
	if (len == 0 || len >= MAX_PATH) {
		return std::string();
	}
	std::string s(path);
	size_t pos = s.find_last_of("\\/");
	if (pos == std::string::npos) {
		return std::string();
	}
	return s.substr(0, pos);
}

std::string JoinPath(const std::string& a, const std::string& b) {
	if (a.empty()) return b;
	const char last = a.back();
	if (last == '\\' || last == '/') return a + b;
	return a + "\\" + b;
}

bool FileExists(const std::string& p) {
	DWORD attr = GetFileAttributesA(p.c_str());
	return attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY) == 0;
}

unsigned long long FileSize(const std::string& p) {
	WIN32_FILE_ATTRIBUTE_DATA data = {};
	if (!GetFileAttributesExA(p.c_str(), GetFileExInfoStandard, &data)) {
		return 0;
	}
	ULARGE_INTEGER li;
	li.HighPart = data.nFileSizeHigh;
	li.LowPart = data.nFileSizeLow;
	return static_cast<unsigned long long>(li.QuadPart);
}

void DeleteIfExists(const std::string& p) {
	if (FileExists(p)) {
		DeleteFileA(p.c_str());
	}
}
} // namespace

TEST(OpenCVFFMpegToolsDll, ExportSymbolsWork) {
	// 这个测试能通过就说明：测试程序能链接并加载 `OPENCVTOOLS.dll`。
	EXPECT_EQ(fnOpenCVTools(), 0);

	void* worker = AvWorker_Create();
	ASSERT_NE(worker, nullptr);
	AvWorker_Destroy(worker);
}

TEST(OpenCVFFMpegToolsDll, GetVideoFirstFrame_GeneratesBmp) {
	const std::string exe_dir = GetExeDir();
	ASSERT_FALSE(exe_dir.empty());

	// 注意：`MultiMediatoolTest.vcxproj` 的 `OutDir` 是 `../bin`，
	// 所以测试运行目录通常就是仓库的 `bin`。
	const std::string input_mp4 = JoinPath(exe_dir, "1.mp4");
	if (!FileExists(input_mp4)) {
		GTEST_SKIP() << "缺少测试输入文件: " << input_mp4;
	}

	const std::string out_bmp = JoinPath(exe_dir, "_test_first_frame.bmp");
	DeleteIfExists(out_bmp);

	void* worker = AvWorker_Create();
	ASSERT_NE(worker, nullptr);

	const bool ok = AvWorker_GetVideoFirstFrame(worker, input_mp4.c_str(), out_bmp.c_str(), false);
	AvWorker_Destroy(worker);

	ASSERT_TRUE(ok);
	ASSERT_TRUE(FileExists(out_bmp));
	ASSERT_GT(FileSize(out_bmp), 0ULL);
}

TEST(OpenCVFFMpegToolsDll, SpliceAV_GeneratesMp4) {
	const std::string exe_dir = GetExeDir();
	ASSERT_FALSE(exe_dir.empty());

	const std::string input1 = JoinPath(exe_dir, "1.mp4");
	const std::string input2 = JoinPath(exe_dir, "2.mp4");
	if (!FileExists(input1) || !FileExists(input2)) {
		GTEST_SKIP() << "缺少测试输入文件: " << input1 << " 或 " << input2;
	}

	const std::string out_mp4 = JoinPath(exe_dir, "_test_splice_out.mp4");
	DeleteIfExists(out_mp4);

	void* worker = AvWorker_Create();
	ASSERT_NE(worker, nullptr);

	const bool ok = AvWorker_SpliceAV(worker, input1.c_str(), input2.c_str(), out_mp4.c_str(), false);
	AvWorker_Destroy(worker);

	ASSERT_TRUE(ok);
	ASSERT_TRUE(FileExists(out_mp4));
	ASSERT_GT(FileSize(out_mp4), 0ULL);
}
