// 612Hw1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include "urlInfo.h"
#include "headerParser.h"

char* winsock_download(const urlInfo& _info);

int main(int argc, char** argv)
{
	std::vector<std::string> urlList;
	//check for only 1 input argument
	if (argc != 2 && argc != 3) {
		printf("You entered %i parameters.\n Usage:\n./<executable> <link>\n or ./<executable> <numThreads> <linkFile>\n", argc - 1);
		exit(-1);
	}
	else if (argc ==2) {
		//printf("Got 1 command line parameter.\n");
		printf("URL: %s\n", argv[1]);
	}
	else if (argc == 3) {
		//currenly only allow 1 thread
		if (strtol(argv[1], nullptr, 10) != 1) {
			std::cerr << "Only allowed 1 thread for now, please change numThreads and rerun.\n";
			exit(-1);
		}
		
		std::string line;
		std::ifstream urlFile(argv[2]);
		if (urlFile.is_open()) {
			while (getline(urlFile, line)) {
				urlList.push_back(line);
			}
			urlFile.close();
		}
		else
			std::cerr << "Unable to open file "<<argv[2]<<"\n";

		for (auto x : urlList) {
			std::cout << x << std::endl;
		}
	}
	
	//need to adapt to using url list and add robots.txt functionality

	int lLen = strlen(argv[1]);
	
	//check url length with max_req_len
	if (lLen > MAX_REQUEST_LEN) {
		std::cerr << "\nURL length is larger than the maximum request length.\n";
		std::cout << argv[1]<<"\n";
		exit(-1);
	}

	//extract and parse info from url
	char* l = new char[lLen + 1];
	memcpy(l, argv[1], lLen + 1);
	urlInfo link;

	link.extract(l);
	printf("\tParsing URL... host %s, port %s, request %s%s\n", link.host, link.port, link.path, link.query);

	delete[] l;
	//check host for max len
	int hLen = strlen(link.host);
	if (hLen > MAX_HOST_LEN) {
		std::cerr << "\nURL host length is larger than the maximum host length.\n";
		std::cout<<link.host<<"\n";
		exit(-1);
	}

	//output url info
	//link.print();

	//do connection and http
	char* httpResponse = winsock_download(link);

	if (httpResponse == nullptr)
		exit(-1);

	//std::cout <<"\"" << httpResponse  <<"\""<< std::endl;

	headerParser p(httpResponse);

	std::cout << "\tVerifying header... status code " << p.statusCode << "\n";

	if (p.statusCode > 199 && p.statusCode < 300) {
		//parse html body here

		HTMLParserBase pb;
		char* pageLinks;
		int nLinks;

		clock_t begin = clock();
		pageLinks = pb.Parse(p.body.string, p.body.length, argv[1], lLen, &nLinks);
		clock_t end = clock();

		printf("      + Parsing page... done in %ims with %i links\n", end - begin, nLinks);
	}
	else {
		//skip html parse
	}

	std::cout << "\n---------------------------------------\n";

	//print header
	printf("%.*s\n", p.header.length, p.header.string);

	free(httpResponse);
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