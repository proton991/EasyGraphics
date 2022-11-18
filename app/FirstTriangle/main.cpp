#include "ezg_engine/engine.hpp"

int main(int argc, char* argv[]) {
  ezg::EGEngine engine;
  engine.Init();
  engine.Run();
  engine.Destroy();
  return 0;
}