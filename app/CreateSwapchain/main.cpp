#include "CreateSwapchain.hpp"

int main(int argc, char* argv[]) {
  ege::EGEngine engine;
  engine.Init();
  engine.DisplayInfo();

  engine.Destroy();
  return 0;
}