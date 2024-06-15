#pragma once

#include <bus/Bus.h>
#include <cpu/ARM.h>

struct ConfigOptions
{
    // Path to the IOSU kernel image, decrypted
    std::string starbuckKernelPath = "fw.img";
};

class WiiU
{
public:
    ~WiiU();

    void OnDestroy();

    void SetConfig(ConfigOptions& options);

    void Init();
    void Run();
private:
    Bus* mBus;
    Starbuck* mArmCpu;
    ConfigOptions options;
};