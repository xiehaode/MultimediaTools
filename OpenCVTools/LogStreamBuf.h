#pragma once
#define  _CRT_SECURE_NO_WARNINGS 1

#include <iostream>
#include <fstream>
#include <streambuf>
#include <string>
#include <ctime>
#include <mutex>
#include <chrono>

// 日志缓冲区类：同时输出到控制台和日志文件
class LogStreamBuf : public std::streambuf {
public:
	
	LogStreamBuf(const std::string& log_file_path)
		: m_console_buf(nullptr), m_file_stream(log_file_path, std::ios::app | std::ios::out) {
		if (m_file_stream.is_open()) {
			// 设置文件流为UTF-8编码（VS2017需此设置支持中文）
			m_file_stream << std::endl; // 先写入空行，避免BOM问题
		}
		
		m_mutex = new std::mutex();
	}

	
	~LogStreamBuf() {
		delete m_mutex;
		if (m_console_buf) {
			
			std::cout.rdbuf(m_console_buf);
			std::cerr.rdbuf(m_console_buf);
		}
		if (m_file_stream.is_open()) {
			m_file_stream.close();
		}
	}

	
	int overflow(int c) override {
		if (c == EOF) {
			return sync(); 
		}

		std::lock_guard<std::mutex> lock(*m_mutex);

		
		if (m_file_stream.is_open()) {
			
			if (m_need_timestamp) {
				add_timestamp();
				m_need_timestamp = false;
			}
			m_file_stream.put(static_cast<char>(c));
			m_file_stream.flush();
		}

		
		if (m_console_buf) {
			m_console_buf->sputc(static_cast<char>(c));
		}

		
		if (c == '\n') {
			m_need_timestamp = true;
		}

		return c;
	}

	
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

	// 换cout/cerr的缓冲区
	void redirect() {
		
		m_console_buf = std::cout.rdbuf();
		// 替换cout和cerr的缓冲区为当前自定义缓冲区
		std::cout.rdbuf(this);
		std::cerr.rdbuf(this);
		// 首次输出需要时间戳
		m_need_timestamp = true;
	}

private:
	// 添加时间戳到日志
	void add_timestamp() {
		auto now = std::chrono::system_clock::now();
		auto time_t_now = std::chrono::system_clock::to_time_t(now);

		// 格式化时间：YYYY-MM-DD HH:MM:SS
		char time_buf[64];
		struct tm time_info {}; // 初始化结构体，避免随机值

		
		errno_t err = localtime_s(&time_info, &time_t_now);
		if (err == 0) { 
			
			strftime(time_buf, sizeof(time_buf), "[%Y-%m-%d %H:%M:%S] ", &time_info);
			m_file_stream << time_buf;
		}
		else {
			
			m_file_stream << "[时间获取失败] ";
		}
	}

	std::streambuf* m_console_buf;       // 保存原有控制台缓冲区
	std::ofstream m_file_stream;         // 日志文件流
	std::mutex* m_mutex;                 // 多线程安全锁
	bool m_need_timestamp = true;        // 是否需要添加时间戳
};