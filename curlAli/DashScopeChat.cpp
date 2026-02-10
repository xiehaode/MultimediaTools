#pragma execution_character_set("utf-8")
#include "pch.h"
#include "DashScopeChat.h"
#include <curl/curl.h>
#include <iostream>
#include <string>
#include <vector>
#include "3rd/nlohmann/nlohmann/json.hpp"
#ifdef _WIN32
#include <windows.h>
#endif


// 回调函数，用于处理 libcurl 接收到的数据
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
	size_t totalSize = size * nmemb;
	static_cast<std::string*>(userp)->append(static_cast<char*>(contents), totalSize);
	return totalSize;
}

#ifdef _WIN32
// 将 UTF-8 字符串转换为本地编码（GBK）
std::string UTF8ToLocal(const std::string& utf8Str)
{
	int len = MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), -1, NULL, 0);
	if (len <= 0) return utf8Str;
	
	wchar_t* wideStr = new wchar_t[len];
	MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), -1, wideStr, len);
	
	int localLen = WideCharToMultiByte(CP_ACP, 0, wideStr, -1, NULL, 0, NULL, NULL);
	if (localLen <= 0) {
		delete[] wideStr;
		return utf8Str;
	}
	
	char* localStr = new char[localLen];
	WideCharToMultiByte(CP_ACP, 0, wideStr, -1, localStr, localLen, NULL, NULL);
	
	std::string result(localStr);
	delete[] wideStr;
	delete[] localStr;
	
	return result;
}
#endif

// 使用 nlohmann JSON 解析响应内容
std::string ExtractContentFromJSON(const std::string& jsonStr)
{
	try {
		nlohmann::json responseJson = nlohmann::json::parse(jsonStr);
		
		// 尝试从不同的响应格式中提取内容
		if (responseJson.contains("choices") && responseJson["choices"].is_array() && 
			!responseJson["choices"].empty() && responseJson["choices"][0].contains("message") &&
			responseJson["choices"][0]["message"].contains("content")) {
			return responseJson["choices"][0]["message"]["content"].get<std::string>();
		}
		
		// 如果是错误响应
		if (responseJson.contains("error") && responseJson["error"].contains("message")) {
			return "Error: " + responseJson["error"]["message"].get<std::string>();
		}
		
		// 如果直接包含 content 字段
		if (responseJson.contains("content")) {
			return responseJson["content"].get<std::string>();
		}
		
		return jsonStr;  // 无法解析，返回原始字符串
	} catch (const std::exception& e) {
		return jsonStr;  // 解析失败，返回原始字符串
	}
}

DashScopeChat::DashScopeChat(const char* apiKey)
	: m_apiKey(apiKey)
{
	// 初始化 libcurl
	curl_global_init(CURL_GLOBAL_ALL);
}

DashScopeChat::~DashScopeChat()
{
	// 清理 libcurl
	curl_global_cleanup();
}

std::string DashScopeChat::chat(const std::string& message)
{
	CURL* curl = curl_easy_init();
	std::string response;

	if (curl) {
		struct curl_slist* headers = nullptr;
		headers = curl_slist_append(headers, "Content-Type: application/json");
		
		std::string auth = "Authorization: Bearer " + m_apiKey;
		headers = curl_slist_append(headers, auth.c_str());

		std::string jsonPayload;
		try {
			// 使用 nlohmann JSON 构建请求体，保持字段顺序
			nlohmann::json requestJson = nlohmann::json::object();
			requestJson["model"] = "qwen-plus";
			
			nlohmann::json messages = nlohmann::json::array();
			
			// 系统消息 - 使用UTF-8字面量确保正确编码
			nlohmann::json systemMsg = nlohmann::json::object();
			systemMsg["role"] = "system";
			// 使用std::string构造函数确保UTF-8编码
			systemMsg["content"] = std::string(u8"你是一个多媒体智能工具辅助机器人，帮助用户写ffmpeg命令来实现他们想要的效果.");
			messages.push_back(systemMsg);
			
			// 用户消息 - 现在有了UTF-8编译器指令，应该能正确处理
			nlohmann::json userMsg = nlohmann::json::object();
			userMsg["role"] = "user";
			
			// 直接使用用户消息
			userMsg["content"] = message;
			messages.push_back(userMsg);
			
			requestJson["messages"] = messages;

			jsonPayload = requestJson.dump(4);  // 缩进4个空格
			
		} catch (const std::exception& e) {
			std::cout << "JSON building error: " << e.what() << std::endl;
			response = "JSON building error: " + std::string(e.what());
			curl_slist_free_all(headers);
			curl_easy_cleanup(curl);
			return response;
		}

		// 调试输出
		std::cout << "=== DEBUG INFO ===" << std::endl;
		std::cout << "JSON Payload: " << jsonPayload << std::endl;
		std::cout << "Payload Length: " << jsonPayload.length() << std::endl;
		std::cout << "=================" << std::endl;

		curl_easy_setopt(curl, CURLOPT_URL, "https://dashscope.aliyuncs.com/compatible-mode/v1/chat/completions");
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonPayload.c_str());
		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, jsonPayload.length());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

		CURLcode res = curl_easy_perform(curl);
		if (res != CURLE_OK) {
			response = "curl error: " + std::string(curl_easy_strerror(res));
		}

		curl_slist_free_all(headers);
		curl_easy_cleanup(curl);
	}

	// 提取内容并编码转换
	std::string content;
	try {
		content = ExtractContentFromJSON(response);
	} catch (const std::exception& e) {
		std::cout << "Content extraction error: " << e.what() << std::endl;
		content = "Error extracting content: " + std::string(e.what());
	}
	
#ifdef _WIN32
	std::string localContent = UTF8ToLocal(content);
	m_lastResponse = localContent;
#else
	m_lastResponse = content;
#endif

	return m_lastResponse;
}

const char* DashScopeChat::getLastResponse() const
{
	return m_lastResponse.c_str();
}



// -------------------- C API 实现 --------------------

extern "C" CURLALI_API void* DashScopeChat_Create(const char* apiKey)
{
	if (!apiKey) return nullptr;
	return new DashScopeChat(apiKey);
}

extern "C" CURLALI_API void DashScopeChat_Destroy(void* chat)
{
	if (chat) {
		delete static_cast<DashScopeChat*>(chat);
	}
}

extern "C" CURLALI_API const char* DashScopeChat_Chat(void* chat, const char* message)
{
	if (!chat || !message) return "";

	DashScopeChat* instance = static_cast<DashScopeChat*>(chat);
	instance->chat(message);
	return instance->getLastResponse();
}


