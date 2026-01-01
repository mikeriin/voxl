// on force l'utilisation des cartes graphiques dédiés, surtout utile pour les laptops hybrides
#ifdef _WIN32
#include <windows.h>

extern "C" 
{
  __declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001; // nvidia
  __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1; // amd
}
#endif


#include "core/engine.h"


int main(int argc, char* argv[])
{
  Engine engine;
  engine.Run();

  return 0;
}