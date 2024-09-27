#pragma once

#include <bus/Bus.h>
#include <cpu/ARM.h>
#include <gpu/GX2.h>
#include <hw/crypto/AES.h>

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

    Starbuck* GetArmCPU() {return mArmCpu;}
    GX2* GetGX2() { return mGX2; }
    AES* GetAES() { return mAES; }
private:
    Bus* mBus;
    Starbuck* mArmCpu;
    GX2* mGX2;
    AES* mAES;
    ConfigOptions options;
};

extern WiiU* gSystem;