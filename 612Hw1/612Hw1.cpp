// 612Hw1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include "urlInfo.h"

void winsock_download(const urlInfo& _info);

int main(int argc, char** argv)
{
	//check for only 1 input argument
	if (argc != 2) {
		printf("You entered %i parameters, please input 1 link.\n", argc - 1);
		exit(-1);
	}
	else {
		//printf("Got 1 command line parameter.\n");
		printf("Retrived 1 Link: %s\n", argv[1]);
	}

	int lLen = strlen(argv[1]);
	
	//check url length with max_req_len
	if (lLen > MAX_REQUEST_LEN) {
		std::cerr << "URL length is larger than the maximum request length.\n";
		std::cout << argv[1]<<"\n";
		exit(-1);
	}

	//extract and parse info from url
	char* l = new char[lLen + 1];
	memcpy(l, argv[1], lLen + 1);
	urlInfo link;
	link.extract(l);

	delete[] l;
	//check host for max len
	int hLen = strlen(link.host);
	if (hLen > MAX_HOST_LEN) {
		std::cerr << "URL host length is larger than the maximum host length.\n";
		std::cout<<link.host<<"\n";
		exit(-1);
	}

	//output url info
	link.print();

	//do connection and http
	winsock_download(link);

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