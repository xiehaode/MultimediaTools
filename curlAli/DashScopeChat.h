#pragma execution_character_set("utf-8")
#pragma once

#include "curlAli.h"
#include <string>
#include "3rd/nlohmann/nlohmann/json.hpp"

/**
 * @brief DashScope 聊天类，模仿 OpenCVTools 的格式
 */
class CURLALI_API DashScopeChat
{
public:
	explicit DashScopeChat(const char* apiKey);
	~DashScopeChat();

	/**
	 * @brief 发送消息并获取回复
	 * @param message 用户消息
	 * @return API 响应字符串
	 */
	std::string chat(const std::string& message);

	/**
	 * @brief 获取最后一次响应
	 * @return 最后一次响应的 C 字符串
	 */
	const char* getLastResponse() const;

private:
	std::string m_apiKey;
	std::string m_lastResponse;
};

