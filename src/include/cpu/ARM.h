#pragma once

#include <cstdint>
#include <bus/Bus.h>
#include <cpu/CP15.h>

enum ArmMode
{
    MODE_USR = 16,
    MODE_FIQ,
    MODE_IRQ,
    MODE_SVC,
    MODE_ABT,
    MODE_UND,
};

class Starbuck
{
    // Public interface
public:
    Starbuck(Bus* pBus);

    void Tick();
    void DumpState();
    // Type definitions
public:
    union PSR
    {
        uint32_t bits;
        struct
        {
            uint32_t m : 5;
            uint32_t t : 1;
            uint32_t f : 1;
            uint32_t i : 1;
            uint32_t a : 1;
            uint32_t e : 1;
            uint32_t : 14;
            uint32_t j : 1;
            uint32_t : 2;
            uint32_t q : 1;
            uint32_t v : 1;
            uint32_t c : 1;
            uint32_t z : 1;
            uint32_t n : 1;
        };
    };
private:
    // State
    uint32_t* mRegs[16];
    uint32_t mNormalRegs[13];
    uint32_t mFiqRegs[7]; // R8-R12, SP, LR
    uint32_t mUsrRegs[2]; // SP, LR
    uint32_t mAbtRegs[2]; // SP, LR
    uint32_t mIrqRegs[2]; // SP, LR
    uint32_t mUndRegs[2]; // SP, LR
    uint32_t mSvcRegs[2]; // SP, LR
    uint32_t mPc;
    PSR cpsr, *curSpsr, svcSpsr;

    bool mDidBranch;
    bool CanDisassemble = false;

    CP15* cp15;

    Bus* mBus;
    // Internal functions
private:
    void SwitchMode(uint32_t mode);
    // ARM mode instructions
private:
    void ExecuteInstruction(uint32_t instr);

    void DoDataProcessing(uint32_t instr);
    void DoLdrStr(uint32_t instr);
    void DoLdmStm(uint32_t instr);
    void DoLdrhStrh(uint32_t instr);
    void DoBranch(uint32_t instr);
    void DoBX(uint32_t instr);
    void DoMCRMRC(uint32_t instr);
    void DoPsrTransfer(uint32_t instr);
    void DoUmull(uint32_t instr);
    void DoMul(uint32_t instr);
    // THUMB mode instructions
private:
    void ExecuteThumbInstruction(uint16_t instr);

    void DoHiRegOp(uint16_t instr);
    void DoAddSub(uint16_t instr);
    void DoRegOpImm(uint16_t instr);
    void DoSvc(uint16_t instr);
    // Helper functions
private:
    inline void SetSZFlags(uint32_t result)
    {
        cpsr.z = (result == 0);
        cpsr.n = (result >> 31) & 1;
    }

    inline void SetCVFlags(uint32_t a, uint32_t b, uint32_t result)
    {
        cpsr.c = ((0xFFFFFFFF-a) < b);
        cpsr.v = (((a) ^ (b)) & 0x80000000) && (((a) ^ (result)) & 0x80000000);
    }

    bool CondPassed(uint8_t cond);
};