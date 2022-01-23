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
	char* l = new char[lLen + 1];
	memcpy(l, argv[1], lLen + 1);
	urlInfo link;
	link.extract(l);
	link.print();

	winsock_download(link);

	delete[] l;
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