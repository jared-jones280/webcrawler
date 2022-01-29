#pragma once
#include "pch.h"
#include "urlInfo.h"
#pragma comment(lib,"ws2_32.lib")

struct winsock {

	constexpr static int INITIAL_BUF_SIZE = 8192;
	constexpr static int THRESHOLD = 1024;

	std::unordered_set<std::string> seenIps;
	std::unordered_set<std::string> seenHosts;

	char* winsock_download(const urlInfo&);

};
