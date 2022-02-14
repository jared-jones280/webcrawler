#pragma once
#include "pch.h"
#include "urlInfo.h"
#include "cStringSpan.h"
#pragma comment(lib,"ws2_32.lib")

struct winstats {
	int nFinished = 0;
	int nHostUnique = 0;
	int nDNSLookup = 0;
	int nIpUnique = 0;
	int nRobotsCheck = 0;
	int nURLs = 0;
	int nLinks = 0;
	int nPageSize = 0;

	int http2 = 0;
	int http3 = 0;
	int http4 = 0;
	int http5 = 0;
	int httpx = 0;
};

struct winsock {

	constexpr static int INITIAL_BUF_SIZE = 2048;
	constexpr static int THRESHOLD = 1024;

	std::mutex mIps;
	std::unordered_set<std::string> seenIps;

	std::mutex mHosts;
	std::unordered_set<std::string> seenHosts;

	cStringSpan readSock(SOCKET, size_t);
	cStringSpan winsock_download(const urlInfo&, size_t, size_t);

	winstats getWinStats();

	bool print = true;
	
	//info and mutexes for stats
	std::mutex mFinished;
	int nFinished = 0;

	std::mutex mHostUnique;
	int nHostUnique = 0;

	std::mutex mDNSLookup;
	int nDNSLookup = 0;
	
	std::mutex mIpUnique;
	int nIpUnique = 0;
	
	std::mutex mRobotsCheck;
	int nRobotsCheck = 0;

	std::mutex mURLs;
	int nURLs = 0;

	std::mutex mLinks;
	int nLinks = 0;

	std::mutex mPageSize;
	int nPageSize = 0;

	std::mutex mHttpCodes;
	int http2 = 0;
	int http3 = 0;
	int http4 = 0;
	int http5 = 0;
	int httpx = 0;

	std::mutex fileWrite;

};

