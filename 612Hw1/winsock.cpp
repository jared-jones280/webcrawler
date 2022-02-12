/* winsock.cpp
by Jared Jones
adapted from winsock.cpp
given by Dmitri Loguinov
*/

#include "pch.h"
#include "winsock.h"
#include "headerParser.h"
#include "cStringSpan.h"

winstats winsock::getWinStats() {
	winstats ret;

	mFinished.lock();
	ret.nFinished = nFinished;
	mFinished.unlock();

	mHostUnique.lock();
	ret.nHostUnique = nHostUnique;
	mHostUnique.unlock();

	mDNSLookup.lock();
	ret.nDNSLookup = nDNSLookup;
	mDNSLookup.unlock();

	mIpUnique.lock();
	ret.nIpUnique = nIpUnique;
	mIpUnique.unlock();

	mRobotsCheck.lock();
	ret.nRobotsCheck = nRobotsCheck;
	mRobotsCheck.unlock();

	mURLs.lock();
	ret.nURLs = nURLs;
	mURLs.unlock();

	return ret;
}

cStringSpan winsock::readSock(SOCKET sock, size_t max_size)
{
	fd_set readFds;
	FD_ZERO(&readFds);
	char* recvbuf = (char*)malloc(INITIAL_BUF_SIZE);
	size_t currBuffSize = INITIAL_BUF_SIZE;
	size_t curr = 0;

	//start timer for download (if > 10s then quit)
	size_t begin = clock();

	while (true) {
		TIMEVAL timeout = { 10,0 };
		FD_ZERO(&readFds);
		FD_SET(sock, &readFds);
		int ret = select(0, &readFds, nullptr, nullptr, &timeout);

		if (ret > 0) {
			int nbytes = recv(sock, &recvbuf[curr], (int)(currBuffSize - curr), 0);

			//checking for slow download there
			size_t end = clock();
			if (end - begin > 10000) {
				if (print) {
					std::cerr << "failed with slow download greater than 10s\n";
				}
				return cStringSpan(nullptr, 0);

			}

			if (nbytes == 0) {
				recvbuf[curr] = '\0';
				break;
			}
			if (nbytes == SOCKET_ERROR) {
				if (print) {
					std::cerr << "\tRecv error occured: " << WSAGetLastError() << "\n";
				}
				free(recvbuf);
				closesocket(sock);
				WSACleanup();
				cStringSpan ret = cStringSpan(nullptr, 0);
				return ret;
			}
			curr += nbytes;
			if (currBuffSize - curr < THRESHOLD) {
				currBuffSize *= 2;

				if (currBuffSize > max_size) {
					if (print) {
						std::cerr << "failed with exceeding max\n";
					}
					cStringSpan ret = cStringSpan(nullptr, 0);
					return ret;

				}
				//char* newbuf = (char*)malloc(currBuffSize);
				//for (int i = 0; i < currBuffSize / 2; i++) {
				//	//std::cout << recvbuf[i];
				//	newbuf[i] = recvbuf[i];
				//}
				//free(recvbuf);
				//recvbuf = newbuf;

				recvbuf = (char*)realloc(recvbuf, currBuffSize*sizeof(char));
				if (recvbuf == nullptr) {
					if (print) {
						std::cerr << "\tMemory allocation error: " << "\n";
					}
					free(recvbuf);
					closesocket(sock);
					WSACleanup();
					return cStringSpan(nullptr,0);
				}
			}
		}
		else if (ret == 0) {
			//timed out
			if (print) {
				std::cerr << "\tRequest timed out\n";
			}
			free(recvbuf);
			closesocket(sock);
			WSACleanup();
			cStringSpan ret = cStringSpan(nullptr, 0);
			return ret;
		}
		else{
			//error
			if (print) {
				std::cerr << "\tSocket error occured: " << WSAGetLastError() << "\n";
			}
			free(recvbuf);
			closesocket(sock);
			WSACleanup();
			cStringSpan ret = cStringSpan(nullptr, 0);
			return ret;
		}
	}
	//for (int i = 0; i < currBuffSize; i++) {
	//	std::cout << recvbuf[i];
	//}
	
	cStringSpan ret = cStringSpan(recvbuf, curr);
	return ret;
}

cStringSpan winsock::winsock_download(const urlInfo& _info, size_t robot_size, size_t page_size)
{
	char* str = _info.host;

	WSADATA wsaData;

	//Initialize WinSock; once per program run
	WORD wVersionRequested = MAKEWORD(2, 2);
	if (WSAStartup(wVersionRequested, &wsaData) != 0) {
		if (print) {
			printf("\tWSAStartup error %d\n", WSAGetLastError());
		}
		WSACleanup();
		return cStringSpan(nullptr,0);
	}

	// open a TCP socket
	SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == INVALID_SOCKET)
	{
		if (print) {
			printf("\tsocket() generated error %d\n", WSAGetLastError());
		}
		WSACleanup();
		return cStringSpan(nullptr,0);
	}

	// structure used in DNS lookups
	struct hostent* remote;

	// structure for connecting to server
	struct sockaddr_in server;

	// first assume that the string is an IP address
	DWORD IP = inet_addr(str);
	bool sHost;
	clock_t begin = clock();
	if (IP == INADDR_NONE)
	{
		// if not a valid IP, then do a DNS lookup
		if ((remote = gethostbyname(str)) == NULL)
		{
			if (print) {
				printf("\tInvalid string: neither FQDN, nor IP address. Failed with %i\n", WSAGetLastError());
			}
			return cStringSpan(nullptr,0);
		}
		else { // take the first IP address and copy into sin_addr
			mHosts.lock();
			auto p = seenHosts.insert(str);
			mHosts.unlock();

			//check Host Uniqueness
			if (p.second) {
				if (print) {
					printf("\tChecking Host uniqueness... passed\n");
				}
			}
			else {
				if (print) {
					printf("\tChecking Host uniqueness... failed\n");
				}
				return cStringSpan(nullptr,0);
			}
			memcpy((char*)&(server.sin_addr), remote->h_addr, remote->h_length);
		}
	}
	else
	{
		if (print) {
			printf("\tChecking Host uniqueness... Skipped (host = IP)\n");
		}
		// if a valid IP, directly drop its binary version into sin_addr
		server.sin_addr.S_un.S_addr = IP;
	}
	clock_t end = clock();

	if (print) {
		printf("\tDoing DNS... done in %ims , found %s\n", end - begin, inet_ntoa(server.sin_addr));
	}

	//check IP uniqueness
	mIps.lock();
	auto p = seenIps.insert(inet_ntoa(server.sin_addr));
	mIps.unlock();
	if (p.second) {
		if (print) {
			printf("\tChecking IP uniqueness... passed\n");
		}
	}
	else {
		if (print) {
			printf("\tChecking IP uniqueness... failed\n");
		}
		return cStringSpan(nullptr,0);
	}

	// setup the port # and protocol type
	server.sin_family = AF_INET;

	const long tmpPort = strtol(_info.port, nullptr, 10);
	if (tmpPort < 0 || tmpPort > 65535) {
		if (print) {
			std::cerr << "\tError: port of out range: " << tmpPort << "\n";
		}
		WSACleanup();
		return cStringSpan(nullptr,0);
	}
	server.sin_port = htons((u_short)tmpPort);		// host-to-network flips the byte order

	////////////////////////////////////robots.txt///////////////////////////////////////
	begin = clock();
	if (connect(sock, (struct sockaddr*)&server, sizeof(struct sockaddr_in)) == SOCKET_ERROR)
	{
		if (print) {
			printf("\tConnection error: %d\n", WSAGetLastError());
		}
		return cStringSpan(nullptr,0);
	}
	end = clock();
	if (print) {
		printf("        Connecting on robots... done in %ims\n", end - begin);
	}

	//create http request
	std::string robots_http_req = "GET robots.txt HTTP/1.0\r\n";
	robots_http_req += "User-agent: tUrTlEcRaWlEr/1.0\r\n";
	robots_http_req += "Host: " + (std::string)_info.host + "\r\n";
	robots_http_req += "Connection: close\r\n";
	robots_http_req += "\r\n";

	begin = clock();
	if (print) {
		printf("\tLoading... ");
	}
	//send http request
	if (SOCKET_ERROR == send(sock, robots_http_req.c_str(), strlen(robots_http_req.c_str()), 0)) {
		if (print) {
			std::cerr << "\tError sending GET request: " << WSAGetLastError() << "\n";
		}
		closesocket(sock);
		WSACleanup();
		return cStringSpan(nullptr,0);
	}

	cStringSpan headbuf = readSock(sock, robot_size);
	if (headbuf.string == nullptr) {
		return cStringSpan(nullptr,0);
	}
	end = clock();

	if (print) {
		printf("done in %ims with %i bytes\n", end - begin, headbuf.length);
	}
	//printf("%s\n", recvbuf);
	// close the socket to this server; open again for the next one
	closesocket(sock);

	//parse robots.txt header

	headerParser hp(headbuf, print);
	if (hp.extract() == 0) {
		return cStringSpan(nullptr,0);
	}

	if (print) {
		std::cout << "\tVerifying header... status code " << hp.statusCode << "\n";
	}

	//looking for DNE (4xx) on robots to continue
	//else continue and parse actual page
	if (hp.statusCode < 400 || hp.statusCode > 499) {
		return cStringSpan(nullptr,0);
	}

	///////////////////////////////////////PAGE//////////////////////////////////////////
	//set up socket again
	// open a TCP socket
	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == INVALID_SOCKET)
	{
		if (print) {
			printf("\tsocket() generated error %d\n", WSAGetLastError());
		}
		WSACleanup();
		return cStringSpan(nullptr,0);
	}

	// connect to the server on port 80
	begin = clock();
	if (connect(sock, (struct sockaddr*)&server, sizeof(struct sockaddr_in)) == SOCKET_ERROR)
	{
		if (print) {
			printf("\tConnection error: %d\n", WSAGetLastError());
		}
		return cStringSpan(nullptr,0);
	}
	end = clock();
	if (print) {
		printf("      * Connecting on page... done in %ims\n", end - begin);

	}

	// send HTTP requests here

	//create http request
	std::string get_http_req = "GET " + (std::string)_info.path + _info.query + " HTTP/1.0\r\n";
	get_http_req += "User-agent: tUrTlEcRaWlEr/1.0\r\n";
	get_http_req += "Host: " + (std::string)_info.host + "\r\n";
	get_http_req += "Connection: close\r\n";
	get_http_req += "\r\n";

	//print for sanity
	//printf("Sending the following request:\n%s", get_http_req.c_str());

	begin = clock();
	if (print) {
		printf("\tLoading... ");
	}
	//send http request
	if (SOCKET_ERROR == send(sock, get_http_req.c_str(), strlen(get_http_req.c_str()), 0)) {
		if (print) {
			std::cerr << "\tError sending GET request: " << WSAGetLastError() << "\n";
		}
		closesocket(sock);
		WSACleanup();
		return cStringSpan(nullptr,0);
	}

	cStringSpan recvbuf = readSock(sock, page_size);

	if (recvbuf.string == nullptr) {
		closesocket(sock);
		WSACleanup();
		return recvbuf;
	}

	end = clock();

	if (print) {
		printf("done in %ims with %lu bytes\n", end - begin, recvbuf.length);
	}

	//printf("%s\n", recvbuf);

	// close the socket to this server; open again for the next one
	closesocket(sock);

	// call cleanup when done with everything and ready to exit program
	WSACleanup();
	//printf("Successfully Disconnected.\n");
	return recvbuf;
}