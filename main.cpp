#include <iostream>
#include <cassert>
#include "vec.h"

int main() {
    vec<int> s;
    s.push_back(12);
    s.push_back(12);
    s.push_back(12);
    s.push_back(12);
    assert(s.size() == 4);
    s.pop_back();
    assert(s.size() == 3);
    std::cout << "pass" << std::endl;
    return 0;
}