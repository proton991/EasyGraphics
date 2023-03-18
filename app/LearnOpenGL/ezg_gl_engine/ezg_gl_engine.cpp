#include "engine/engine.hpp"

using namespace ezg::system;
using namespace ezg::gl;

extern "C" {
__declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
}

int main() {
  Engine engine;
  engine.initialize("SimpleScene");
  engine.run();
  return 0;
}