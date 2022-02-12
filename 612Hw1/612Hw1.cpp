// 612Hw1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include "urlInfo.h"
#include "headerParser.h"
#include "winsock.h"
#include "threadSafeQueue.h"

int consume(threadSafeQueue* urlList, winsock * w, bool print) {
	while(urlList->size() > 0) {
		auto x = urlList->pop();
		if (print) {
			printf("[%i]URL: %s\n",std::this_thread::get_id(), x.c_str());
		}
		//need to adapt to using url list and add robots.txt functionality

		int lLen = strlen(x.c_str());

		//check url length with max_req_len
		if (lLen > MAX_REQUEST_LEN) {
			if (print) {
				std::cerr << "\nURL length is larger than the maximum request length.\n";
				std::cout << x << "\n";
			}
			continue;
		}

		//extract and parse info from url
		char* l = new char[lLen + 1];
		memcpy(l, x.c_str(), lLen + 1);
		urlInfo link;
		link.print = print;

		if (link.extract(l) == 0) {
			continue;
		}
		if (print) {
			printf("\tParsing URL... host %s, port %s, request %s%s\n", link.host, link.port, link.path, link.query);
		}

		delete[] l;
		//check host for max len
		int hLen = strlen(link.host);
		if (hLen > MAX_HOST_LEN) {
			if (print) {
				std::cerr << "\nURL host length is larger than the maximum host length.\n";
				std::cout << link.host << "\n";
			}
			continue;
		}

		//output url info
		//link.print();

		//do connection and http
		//specifiying 16kb for max robots size and 2mb for max page size.
		cStringSpan httpResponse = w->winsock_download(link, 16384, 2097152);

		if (httpResponse.string == nullptr)
			continue;

		//std::cout <<"\"" << httpResponse  <<"\""<< std::endl;

		headerParser p(httpResponse, print);
		if (p.extract() == 0) {
			continue;
		}

		if (print) {
			std::cout << "\tVerifying header... status code " << p.statusCode << "\n";
		}

		if (p.statusCode > 199 && p.statusCode < 300) {
			//parse html body here

			HTMLParserBase pb;
			char* pageLinks;
			int nLinks;

			char* savedLink = new char[lLen+1];
			strcpy_s(savedLink,lLen+1,x.c_str());
			clock_t begin = clock();
			pageLinks = pb.Parse(p.body.string, httpResponse.length, savedLink, lLen, &nLinks);
			clock_t end = clock();
			//printf("%s", savedLink);
			delete[] savedLink;

			if (print) {
				printf("      + Parsing page... done in %ims with %i links\n", end - begin, nLinks);
			}
		}
		else {
			//skip html parse
		}

		//std::cout << "\n---------------------------------------\n";

		//print header
		//printf("%.*s\n", p.header.length, p.header.string);

		free(httpResponse.string);
	}

	//increment nFinished because 1 consumer finished
	w->mFinished.lock();
	w->nFinished++;
	w->mFinished.unlock();

	return 0;
}

int stats(threadSafeQueue* urlList, winsock* w, int nThreads, int initListSize) {
	winstats s = w->getWinStats();
	int time = 0;
	while (s.nFinished < nThreads) {
		s = w->getWinStats();
		//time (sec), number of active remaining threads, size of queue
		int currList = urlList->size();
		printf("[%3d] %5i Q%6d E%7d\n", time, nThreads - s.nFinished, currList, initListSize-currList);
		
		
		time += 2;
		Sleep(2000);
	}

	return 0;
}

// TODO: allow for optional printing output and stats thread that tracks stats
// write report
int main(int argc, char** argv)
{
	//std::vector<std::string> urlList;
	threadSafeQueue urlList;
	int nthreads=1;
	bool print = true;

	//check for only 1 input argument
	if (argc != 2 && argc != 3) {
		printf("You entered %i parameters.\n Usage:\n./<executable> <link>\n or ./<executable> <numThreads> <linkFile>\n", argc - 1);
		exit(-1);
	}
	else if (argc ==2) {
		//printf("Got 1 command line parameter.\n");
		urlList.push(argv[1]);
	}
	else if (argc == 3) {
		nthreads = strtol(argv[1], nullptr,10);
		print = false;
		
		std::string line;
		std::ifstream urlFile(argv[2]);
		if (urlFile.is_open()) {
			std::streampos begin = urlFile.tellg();
			urlFile.seekg(0, std::ios::end);
			std::streampos end = urlFile.tellg();
			urlFile.seekg(std::ios::beg);
			while(getline(urlFile, line)) {
				urlList.push(line);
			}
			urlFile.close();
			printf("Opened %s with size %i\n", argv[2], (end-begin));
		}
		else
			std::cerr << "Unable to open file "<<argv[2]<<"\n";
	}

	winsock w;
	w.print = print;

	//if not outputting individual website info / input then create stats thread
	std::vector<std::thread> stat;
	if (!print) {
		stat.push_back(std::thread(stats, &urlList, &w, nthreads, urlList.size()));
	}

	//lets make some threads
	std::vector<std::thread> consumers;
	for (int i = 0; i < nthreads; i++) {
		consumers.push_back(std::thread(consume, &urlList, &w, print));
	}

	//wait for them all to join
	for (auto &x : consumers) {
		x.join();
	}

	//join stat thread if not printing normal output
	if (!print) {
		for (auto& x : stat) {
			x.join();
		}
	}

	return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started:
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file