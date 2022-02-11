#pragma once
#include "pch.h"
#include "urlInfo.h"
#include "cStringSpan.h"
#pragma comment(lib,"ws2_32.lib")

struct winsock {

	constexpr static int INITIAL_BUF_SIZE = 2048;
	constexpr static int THRESHOLD = 1024;

	std::unordered_set<std::string> seenIps;
	std::unordered_set<std::string> seenHosts;

	cStringSpan readSock(SOCKET, size_t);
	cStringSpan winsock_download(const urlInfo&, size_t, size_t);

	std::mutex mIps;
	std::mutex mHosts;

};
