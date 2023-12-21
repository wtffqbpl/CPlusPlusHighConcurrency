#include <iostream>
#include <thread>

// CppMem is an interactive tool for exploring the behaviour of small
// code snippets using the C++ memory model.

int x = 0;
int y = 0;

void writing() {
  x = 2000;
  y = 11;
}

void reading() {
  std::cout << "y: " << y << " "
            << "x: " << x << std::endl;
}
