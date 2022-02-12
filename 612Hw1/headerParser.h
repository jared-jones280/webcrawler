#pragma once
#include "cStringSpan.h"


struct headerParser {
	
	char* http;
	int olen;
	cStringSpan header;
	cStringSpan body;
	int statusCode;
	bool print=true;
		

	headerParser(cStringSpan _http, bool _print) {
		http = _http.string;
		olen = _http.length;
		print = _print;
	}
	
	bool extract() {
		//some quick error checkin

		if (strstr(http, "\r\n\r\n") == nullptr || strstr(http, "HTTP") == nullptr) {

			if (print) {
				printf("failed with non-HTTP header\n");
			}
			return 0;
		}

		//parse out header
		char* eoh = strstr(http, "\r\n\r\n");
		header.string = http;
		header.length = eoh - http;

		//body
		body.string = eoh + 4;
		body.length = olen-header.length;

		std::regex sCode("\\d\\d\\d");
		std::cmatch matches;
		if (std::regex_search(header.string,matches,sCode)) {
			//first match will be status code
			statusCode = strtol(matches[0].first, nullptr, 10);
			return 1;
		}
		else {
			if (print) {
				std::cerr << "Unable to find status code in html response, this is not valid\n";
			}
			return 0;
		}

	}

};
