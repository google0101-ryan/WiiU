#include <system/WiiU.h>
#include <memory>
#include <csignal>

WiiU* gSystem;

void sig(int)
{
    gSystem->OnDestroy();
}

int main(int argc, char** argv)
{
    WiiU system;
    system.Init();

    gSystem = &system;

    signal(SIGINT, sig);
    signal(SIGABRT, sig);

    while (true)
    {
        try
        {
            system.Run();
        }
        catch (std::exception& e)
        {
            printf("***************************************************************************************\n");
            printf("*                                      ERROR                                          *\n");
            printf("***************************************************************************************\n");
            printf("%s\n", e.what());
            return -1;
        }
    }

    return 0;
}