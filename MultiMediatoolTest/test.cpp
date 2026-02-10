#include "pch.h"
#include "../OpenCVTools/OpenCVFFMpegTools.h"
#include "../formatChange/formatChange.h"
#include "../curlAli/curlAli.h"
#include <string>
#include <iostream>
#include <Windows.h>
#include "lan_util.h"




int main() {
	void * chater = DashScopeChat_Create("sk-a7747b9ab5954cfdb2ff5ddc3cc6946a");
	const char* response = DashScopeChat_Chat(chater,gbk_to_utf8("表演一下才艺").c_str());
	std::cout << "AI Response: " << response << std::endl;
	
	DashScopeChat_Destroy(chater);
	return 0;
}