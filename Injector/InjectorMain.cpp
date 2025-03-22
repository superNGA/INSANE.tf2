#include <iostream>
#include <string>
#include "InjectorLogic.h"

const char* TARGET_PROCESS  = "tf_win64.exe";
const char* DLL_PATH        = "..\\output\\INSANE.tf2.dll";
int main(void)
{
    if (injector.Intitialize(TARGET_PROCESS, DLL_PATH) == false)
    {
        std::cout << "Failed intitilization\n";
        return 0;
    }

    if (injector.inject(injectionConfig_t::MANUAL_MAP) == false)
    {
        std::cout << "Failed injection\n";
        return 0;
    }

    std::cout << "injection done\n";
    
    return 0;
}