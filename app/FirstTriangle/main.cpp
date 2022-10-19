#include "eg_engine/engine.hpp"

int main(int argc, char* argv[]) {
  ege::EGEngine engine;
  engine.Init();
  engine.Run();
  engine.Destroy();
  return 0;
}