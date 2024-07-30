#include <iostream>

int main() {
    std::cout << "This is a test program for detecting indent, blah blah";
    for (int i = 0; i < 10; ++i) {
        if (i == 2) {
            i++;
        } else {
            i--;
        }
    }
    return 1;
}
