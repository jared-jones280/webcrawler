/* winsock.cpp
by Jared Jones
adapted from winsock.cpp
given by Dmitri Loguinov
*/

#include "pch.h"
#include "urlInfo.h"
#pragma comment(lib,"ws2_32.lib")

constexpr int INITIAL_BUF_SIZE = 8192;
constexpr int THRESHOLD = 1024;

void winsock_download(const urlInfo& _info)
{
	char* str = _info.host;

	WSADATA wsaData;

	//Initialize WinSock; once per program run
	WORD wVersionRequested = MAKEWORD(2, 2);
	if (WSAStartup(wVersionRequested, &wsaData) != 0) {
		printf("WSAStartup error %d\n", WSAGetLastError());
		WSACleanup();
		return;
	}

	// open a TCP socket
	SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == INVALID_SOCKET)
	{
		printf("socket() generated error %d\n", WSAGetLastError());
		WSACleanup();
		return;
	}

	// structure used in DNS lookups
	struct hostent* remote;

	// structure for connecting to server
	struct sockaddr_in server;

	// first assume that the string is an IP address
	DWORD IP = inet_addr(str);
	if (IP == INADDR_NONE)
	{
		// if not a valid IP, then do a DNS lookup
		if ((remote = gethostbyname(str)) == NULL)
		{
			printf("Invalid string: neither FQDN, nor IP address\n");
			return;
		}
		else // take the first IP address and copy into sin_addr
			memcpy((char*)&(server.sin_addr), remote->h_addr, remote->h_length);
	}
	else
	{
		// if a valid IP, directly drop its binary version into sin_addr
		server.sin_addr.S_un.S_addr = IP;
	}
	// setup the port # and protocol type
	server.sin_family = AF_INET;

	const long tmpPort = strtol(_info.port, nullptr, 10);
	if (tmpPort < 0 || tmpPort > 65535) {
		std::cerr << "Error: port of out range: " << tmpPort << "\n";
		WSACleanup();
		return;
	}
	server.sin_port = htons((u_short)tmpPort);		// host-to-network flips the byte order

	// connect to the server on port 80
	if (connect(sock, (struct sockaddr*)&server, sizeof(struct sockaddr_in)) == SOCKET_ERROR)
	{
		printf("Connection error: %d\n", WSAGetLastError());
		return;
	}

	printf("Successfully connected to %s (%s) on port %d\n", str, inet_ntoa(server.sin_addr), htons(server.sin_port));

	// send HTTP requests here

	//create http request
	std::string get_http_req = "GET " + (std::string)_info.path + _info.query + " HTTP/1.0\r\n";
	get_http_req += "User-agent: tUrTlEcRaWlEr/1.0\r\n";
	get_http_req += "Host: " + (std::string)_info.host + "\r\n";
	get_http_req += "Connection: close\r\n";
	get_http_req += "\r\n";

	//print for sanity
	printf("Sending the following request:\n%s", get_http_req.c_str());

	//send http request
	if (SOCKET_ERROR == send(sock, get_http_req.c_str(), strlen(get_http_req.c_str()), 0)) {
		std::cerr << "Error sending GET request: " << WSAGetLastError() << "\n";
		closesocket(sock);
		WSACleanup();
		exit(-1);
	}

	fd_set readFds;
	FD_ZERO(&readFds);
	char* recvbuf = (char*)malloc(INITIAL_BUF_SIZE);
	size_t currBuffSize = INITIAL_BUF_SIZE;
	size_t curr = 0;

	while (true) {
		TIMEVAL timeout = { 10,0 };
		FD_SET(sock, &readFds);
		int ret = select(0, &readFds, nullptr, nullptr, &timeout);

		if (ret > 0) {
			// do recv
			int nbytes = recv(sock, &recvbuf[curr], (int)(currBuffSize - curr), 0);

			if (nbytes == 0) {
				recvbuf[curr] = '\0';
				break;
			}
			if (nbytes == SOCKET_ERROR) {
				std::cerr << "Recv error occured: " << WSAGetLastError() << "\n";
				free(recvbuf);
				closesocket(sock);
				WSACleanup();
				return;
			}

			curr += nbytes;
			if (currBuffSize - curr < THRESHOLD) {
				currBuffSize *= 2;
				char* tmpBuf = (char*)realloc(recvbuf, currBuffSize);
				if (tmpBuf == nullptr) {
					std::cerr << "Memory allocation error: " << "\n";
					free(recvbuf);
					closesocket(sock);
					WSACleanup();
					return;
				}
				recvbuf = tmpBuf;
			}
		}
		else if (ret == 0) {
			//timed out
			std::cerr << "Request timed out\n";
			free(recvbuf);
			closesocket(sock);
			WSACleanup();
			return;
		}
		else if (ret == SOCKET_ERROR) {
			//error
			std::cerr << "Socket error occured: " << WSAGetLastError() << "\n";
			free(recvbuf);
			closesocket(sock);
			WSACleanup();
			return;
		}
	}

	printf("%s\n", recvbuf);

	// close the socket to this server; open again for the next one
	closesocket(sock);

	// call cleanup when done with everything and ready to exit program
	WSACleanup();
	printf("Successfully Disconnected, exiting cleanly.\n");
}