#include <system/WiiU.h>

WiiU::~WiiU()
{
    OnDestroy();
}

void WiiU::OnDestroy()
{
    if (mArmCpu && mBus)
    {
        mArmCpu->DumpState();
        mBus->Dump();

        delete mArmCpu;
        delete mBus;
    }
}

void WiiU::SetConfig(ConfigOptions &options)
{
    this->options = options;
}

void WiiU::Init()
{
    mBus = new Bus(options.starbuckKernelPath);
    mArmCpu = new Starbuck(mBus);
    mGX2 = new GX2();
    mAES = new AES();
}

void WiiU::Run()
{
    mBus->Update();
    mArmCpu->Tick();
}
