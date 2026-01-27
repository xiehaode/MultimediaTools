#pragma once
#define  _CRT_SECURE_NO_WARNINGS 1

#include <iostream>
#include <fstream>
#include <streambuf>
#include <string>
#include <ctime>
#include <mutex>
#include <chrono>

// 自定义日志缓冲区类：同时输出到控制台和日志文件
class LogStreamBuf : public std::streambuf {
public:
	// 构造函数：指定日志文件路径
	LogStreamBuf(const std::string& log_file_path)
		: m_console_buf(nullptr), m_file_stream(log_file_path, std::ios::app | std::ios::out) {
		if (m_file_stream.is_open()) {
			// 设置文件流为UTF-8编码（VS2017需此设置支持中文）
			m_file_stream << std::endl; // 先写入空行，避免BOM问题
		}
		// 互斥锁保证多线程安全
		m_mutex = new std::mutex();
	}

	// 析构函数：恢复缓冲区
	~LogStreamBuf() {
		delete m_mutex;
		if (m_console_buf) {
			// 恢复控制台缓冲区
			std::cout.rdbuf(m_console_buf);
			std::cerr.rdbuf(m_console_buf);
		}
		if (m_file_stream.is_open()) {
			m_file_stream.close();
		}
	}

	// 重写overflow：处理单个字符输出
	int overflow(int c) override {
		if (c == EOF) {
			return sync(); // 刷新缓冲区
		}

		std::lock_guard<std::mutex> lock(*m_mutex);

		// 1. 写入日志文件（带时间戳）
		if (m_file_stream.is_open()) {
			// 首次写入时添加时间戳
			if (m_need_timestamp) {
				add_timestamp();
				m_need_timestamp = false;
			}
			m_file_stream.put(static_cast<char>(c));
			m_file_stream.flush();
		}

		// 2. 输出到控制台（保留原有cout/cerr行为）
		if (m_console_buf) {
			m_console_buf->sputc(static_cast<char>(c));
		}

		// 换行时标记下一次需要添加时间戳
		if (c == '\n') {
			m_need_timestamp = true;
		}

		return c;
	}

	// 重写sync：刷新缓冲区
	int sync() override {
		std::lock_guard<std::mutex> lock(*m_mutex);
		if (m_file_stream.is_open()) {
			m_file_stream.flush();
		}
		if (m_console_buf) {
			m_console_buf->pubsync();
		}
		return 0;
	}

	// 初始化：替换cout/cerr的缓冲区
	void redirect() {
		// 保存原有控制台缓冲区
		m_console_buf = std::cout.rdbuf();
		// 替换cout和cerr的缓冲区为当前自定义缓冲区
		std::cout.rdbuf(this);
		std::cerr.rdbuf(this);
		// 首次输出需要时间戳
		m_need_timestamp = true;
	}

private:
	// 添加时间戳到日志（修复localtime不安全问题）
	void add_timestamp() {
		auto now = std::chrono::system_clock::now();
		auto time_t_now = std::chrono::system_clock::to_time_t(now);

		// 格式化时间：YYYY-MM-DD HH:MM:SS
		char time_buf[64];
		struct tm time_info {}; // 初始化结构体，避免随机值

		// 完全使用安全的localtime_s，替代不安全的localtime
		errno_t err = localtime_s(&time_info, &time_t_now);
		if (err == 0) { // 确保localtime_s调用成功
			// 直接使用已填充的time_info，不再调用localtime
			strftime(time_buf, sizeof(time_buf), "[%Y-%m-%d %H:%M:%S] ", &time_info);
			m_file_stream << time_buf;
		}
		else {
			// 异常处理：localtime_s调用失败时输出默认标识
			m_file_stream << "[时间获取失败] ";
		}
	}

	std::streambuf* m_console_buf;       // 保存原有控制台缓冲区
	std::ofstream m_file_stream;         // 日志文件流
	std::mutex* m_mutex;                 // 多线程安全锁
	bool m_need_timestamp = true;        // 是否需要添加时间戳
};