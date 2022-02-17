#include "naitives.hpp"


value_t clockNative(int argCount, stack_iter args) {
  return NUMBER_VAL((double)clock() / CLOCKS_PER_SEC);
}