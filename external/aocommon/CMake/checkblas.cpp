#include <aocommon/checkblas.h>

#include <iostream>

int main() {
  try {
    check_openblas_multithreading();
    return 0;
  } catch (std::runtime_error& e) {
    std::cout << e.what() << '\n';
    return 1;
  }
}
