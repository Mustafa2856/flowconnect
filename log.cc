#include <iostream>

#define LOGGER_INCLUDED

bool verbose = false;

void log(){
	if(!verbose)return;
	std::cout<<"\n";
}

template<typename First, typename ...Rest>
void log(First && first, Rest && ...rest) {
	if(!verbose)return;
	std::cout << std::forward<First>(first)<<" ";
	log(std::forward<Rest>(rest)...);
}

