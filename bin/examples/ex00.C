#include <iostream>
#include <stdexcept>
#include <hobbes/hobbes.H>

int applyDiscreteWeighting(int x, int y) { return x * y; }

int main() {
  hobbes::cc c;
  // c.bind("applyDiscreteWeighting", &applyDiscreteWeighting);
  std::string x = "abcdefghijklmnopqrstuvwxyz";
  std::cout
  << c.compileFn<bool(std::string*)>("x", "x[0:2] == \"ab\"")(&x)
  << c.compileFn<bool(std::string*)>("x", "x[2:0] == \"ba\"")(&x)
  << c.compileFn<bool(std::string*)>("x", "x[-5:-1] == \"vwxy\"")(&x)
  << c.compileFn<bool(std::string*)>("x", "x[-5:] == \"vwxyz\"")(&x)
  << c.compileFn<bool(std::string*)>("x", "x[:-10] == \"zyxwvutsrq\"")(&x)
  << c.compileFn<bool(std::string*)>("x", "x[0:0] == \"\"")(&x)
  << c.compileFn<bool(std::string*)>("x", "length(x[:-10]) == 10L")(&x)
  << std::endl;
  return 0;
}