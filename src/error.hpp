#ifndef ERROR_HPP
#define ERROR_HPP

#include <iostream>
#include <string_view>
#include <cstdlib>

[[noreturn]] inline void die(std::string_view msg){
    std::cerr << msg << "\n";
    std::exit(EXIT_FAILURE);
}

#endif