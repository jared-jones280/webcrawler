#pragma once
#include "pch.h"

struct urlInfo {
	char* start = nullptr;
	char* link = nullptr;
	char* httpCheck = nullptr;
	char* host = nullptr;
	char* query = nullptr;
	char* path = nullptr;
	char* port = nullptr;

	void extract(char* _a) {
		start = _a;
		httpCheck = strstr(start, "http://");
		if (httpCheck == nullptr) {
			printf("Could not find \'http://\' in link. Make sure your link is http.\n");
			exit(-1);
		}
		else {
			//printf("Found \'http://\'\n");
		}

		link = start + 7;

		//1. Find # using strrchr() (finds last) and truncate
		char* pound = strrchr(link, '#');
		if (pound != nullptr) {
			link[pound - link] = '\0';
			//printf("Truncated frag: %s\n", link);
		}

		//2. Find ?, extract query, truncate
		char* q = strrchr(link, '?');
		if (q != nullptr) {
			int qlen = strlen(q);
			query = new char[qlen + 1];
			memcpy(query, q, qlen + 1);
			link[q - link] = '\0';
			//printf("Truncated query: %s\n", link);
		}
		else {
			query = new char[1];
			memcpy(query, "\0", 1);
		}
		//3. Find /, extract path, truncate
		char* pa = strchr(link, '/');
		if (pa != nullptr) {
			int palen = strlen(pa);
			path = new char[palen + 1];
			memcpy(path, pa, palen + 1);
			link[pa - link] = '\0';
			//printf("Truncated path: %s\n", link);
		}
		else {
			path = new char[2];
			memcpy(path, "/", 2);
		}
		//4. Find :, extract port, truncate, obtain host
		char* p = strchr(link, ':');
		if (p != nullptr) {
			int plen = strlen(p);
			port = new char[plen];
			memcpy(port, p+1, plen);
			link[p - link] = '\0';
			//printf("Truncated port : % s\n", link);
			//printf("Port: %s\n", port);
		}
		else {
			port = new char[3];
			memcpy(port, "80", 3);
		}
		//obtain host
		int hlen = strlen(link) + 1;
		host = new char[hlen];
		memcpy(host, link, hlen);
	}

	void printIndividual(char* dat) {
		if (dat == nullptr) {
			printf("-Not Found-\n");
		}
		else {
			printf("%s\n", dat);
		}
	}

	void print() {
		printf("Host: ");
		this->printIndividual(host);
		printf("Port: ");
		this->printIndividual(port);
		printf("Path: ");
		this->printIndividual(path);
		printf("Query: ");
		this->printIndividual(query);
	}

	~urlInfo() {
		delete[] host;
		delete[] query;
		delete[] path;
		delete[] port;
	}
};
