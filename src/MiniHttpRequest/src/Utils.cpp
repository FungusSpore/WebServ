#include "Utils.hpp"
#include <iostream>
#include <sstream>

void ft_strtrim(std::string& s) {
	size_t start = s.find_first_not_of(M_WS);
	size_t end = s.find_last_not_of(M_WS);
	
	if (start == std::string::npos || end == std::string::npos) {
		s.clear();
	} else {
		s = s.substr(start, end - start + 1);
	}
}
