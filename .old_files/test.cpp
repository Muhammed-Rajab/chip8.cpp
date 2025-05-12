#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <sys/types.h>

#define LOG std::cout
#define NL "\n"

unsigned int hexToDecimal(std::string hex) {

  int power = 0;
  unsigned int acc = 0;
  size_t size = hex.size();

  for (int index = size - 1; index >= 0; index -= 1) {
    char ch = hex.at(index);

    if (ch == 'x' || ch == 'X')
      return acc;

    if (ch >= '0' && ch <= '9')
      acc += (ch - '0') * std::pow(16, power);
    else if (ch >= 'A' && ch <= 'F')
      acc += (ch - 'A' + 10) * std::pow(16, power);
    else if (ch >= 'a' && ch <= 'f')
      acc += (ch - 'a' + 10) * std::pow(16, power);
    else
      throw std::invalid_argument("invalid hex digit");

    power += 1;
  }

  return acc;
}

std::string decimalToHex(unsigned int num) {

  if (num == 0)
    return "0x0";

  int rem = 0;
  int quotient = 0;

  std::string hex;

  while (num != 0) {
    quotient = num / 16;
    rem = num % 16;

    if (rem >= 0 && rem <= 9)
      hex.push_back('0' + rem);
    else if (rem >= 10 && rem <= 15)
      hex.push_back('A' + rem - 10);
    else
      throw std::invalid_argument("invalid decimal argument");

    num = quotient;
  }

  hex.push_back('x');
  hex.push_back('0');

  std::reverse(hex.begin(), hex.end());

  return hex;
}

void bitwiseFun() {

  // AND(&) OPERATION
  char a = 7; // 0000 0111
  char b = 6; // 0000 0110

  // 0000 0111
  //         &
  // 0000 0110
  // ---------
  // 0000 0110 ---> '6' in decimal

  LOG << (a & b) << NL;

  // OR(|) OPERATION
  // 0000 0111
  //         |
  // 0000 0110
  // ---------
  // 0000 0111 ---> '7' in decimal

  // NOT(~) OPERATOR
  // ~(0000 0111) is 1111 1000 -> -8
  LOG << (unsigned int)~a << NL;

  LOG << (~a + 1) << NL;

  unsigned char X = -5;
  LOG << (~X + 1) << NL;
}

void bitwiseOperationsMore() {
  int x = 10;
  LOG << ((x >> (sizeof(x) * 8 - 1)) & 1) << NL;

  unsigned int h = 0;
  LOG << ~h << NL;
}

int main() {
  bitwiseOperationsMore();
  return EXIT_SUCCESS;
}
