#pragma once
#include "pch.h"
class IRecordStrategy {
public:
	virtual ~IRecordStrategy() = default;
	virtual bool Init(const std::wstring& title, int width, int height) = 0;
	virtual bool Start(const std::string& fileName) = 0;
	virtual bool Stop() = 0;
};

// GDI录制类（内部，不对外暴露）
class GDIRecord : public IRecordStrategy {
public:
	GDIRecord() = default;
	~GDIRecord() override;
	bool Init(const std::wstring& title, int width, int height) override;
	bool Start(const std::string& fileName) override;
	bool Stop() override;

private:
	HWND m_hWnd = nullptr;
	AVFormatContext* m_fmtCtx = nullptr;
	AVCodecContext* m_codecCtx = nullptr;
};

