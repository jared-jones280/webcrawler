#pragma once

struct cStringSpan {
	char* string;
	int length;

	cStringSpan() {
		string = nullptr;
		length = 0;
	}

	cStringSpan(char* _string, int _length) {
		string = _string;
		length = _length;
	}
};
