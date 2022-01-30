#pragma once

struct cStringSpan {
	char* string;
	int length;
};

struct headerParser {
	
	char* http;
	cStringSpan header;
	cStringSpan body;
	int statusCode;
		

	headerParser(char* _http) {
		http = _http;
	}
	
	bool extract() {
		//some quick error checkin

		if (strstr(http, "\r\n\r\n") == nullptr || strstr(http, "HTTP") == nullptr) {
			printf("failed with non-HTTP header\n");
			return 0;
		}

		//parse out header
		char* eoh = strstr(http, "\r\n\r\n");
		header.string = http;
		header.length = eoh - http;

		//body
		body.string = eoh + 4;
		body.length = strlen(body.string);

		std::regex sCode("\\d\\d\\d");
		std::cmatch matches;
		if (std::regex_search(header.string,matches,sCode)) {
			//first match will be status code
			statusCode = strtol(matches[0].first, nullptr, 10);
			return 1;
		}
		else {
			std::cerr << "Unable to find status code in html response, this is not valid\n";
			return 0;
		}

	}

};
