#pragma once

#include <cstdint>
#include <bus/Bus.h>
#include <cpu/CP15.h>

enum ArmMode
{
    MODE_USR = 0x10,
    MODE_FIQ = 0x11,
    MODE_IRQ = 0x12,
    MODE_SVC = 0x13,
    MODE_ABT = 0x17,
    MODE_UND = 0x1B,
    MODE_SYS = 0x1F
};

class Starbuck
{
    friend class CP15;
    // Public interface
public:
    Starbuck(Bus* pBus);

    void DoInterrupt();

    void Tick();
    void DumpState();

    uint32_t GetReg(int reg)
    {
        return (*mRegs[reg]);
    }

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
    PSR cpsr, *curSpsr, fiqSpsr, svcSpsr, abtSpsr, irqSpsr, undSpsr;

    bool mDidBranch;
    bool CanDisassemble = false;

    CP15* cp15;

    Bus* mBus;
    // Internal functions
private:
    void SwitchMode(uint32_t mode);

    uint32_t TranslateAddr(uint32_t addr, bool isWrite);

    uint32_t Read32(uint32_t addr);
    uint16_t Read16(uint32_t addr);
    uint8_t Read8(uint32_t addr);

    void Write32(uint32_t addr, uint32_t data);
    void Write16(uint32_t addr, uint16_t data);
    void Write8(uint32_t addr, uint8_t data);
    // ARM mode instructions
private:
    void ExecuteInstruction(uint32_t instr);

    void DoDataProcessing(uint32_t instr);
    void DoLdrStr(uint32_t instr);
    void DoLdmStm(uint32_t instr);
    void DoBlockLoad(uint32_t instr);
    void DoBlockStore(uint32_t instr);
    void DoLdrhStrh(uint32_t instr);
    void DoBranch(uint32_t instr);
    void DoUndefined(uint32_t instr);
    void DoBX(uint32_t instr);
    void DoMCRMRC(uint32_t instr);
    void DoCLZ(uint32_t instr);
    void DoPsrTransfer(uint32_t instr);
    void DoUmull(uint32_t instr);
    void DoSmull(uint32_t instr);
    void DoMul(uint32_t instr);
    void DoMla(uint32_t instr);
    // THUMB mode instructions
private:
    void ExecuteThumbInstruction(uint16_t instr);

    void DoHiRegOp(uint16_t instr);
    void DoAddSub(uint16_t instr);
    void DoAddSp(uint16_t instr);
    void DoRegOpImm(uint16_t instr);
    void DoGetReladdr(uint16_t instr);
    void DoSvc(uint16_t instr);
    void DoCondBranch(uint16_t instr);
    void DoLongBL1(uint16_t instr);
    void DoLongBL2(uint16_t instr);
    void DoPushPop(uint16_t instr);
    void DoLoadStoreSpRel(uint16_t instr);
    void DoLoadPcRel(uint16_t instr);
    void DoLoadStoreImm(uint16_t instr);
    // Helper functions
private:
    inline void SetSZFlags(uint32_t result)
    {
        cpsr.z = (result == 0);
        cpsr.n = (result >> 31) & 1;
    }

    #define CARRY_ADD(a, b)  ((0xFFFFFFFF-a) < b)
    #define CARRY_SUB(a, b)  (a >= b)

    #define ADD_OVERFLOW(a, b, result) ((!(((a) ^ (b)) & 0x80000000)) && (((a) ^ (result)) & 0x80000000))
    #define SUB_OVERFLOW(a, b, result) (((a) ^ (b)) & 0x80000000) && (((a) ^ (result)) & 0x80000000)

    inline void SetCVFlags(uint32_t a, uint32_t b, uint32_t result)
    {
        cpsr.c = CARRY_ADD(a, b);
        cpsr.v = ADD_OVERFLOW(a, b, result);
    }

    inline void SetCVSubFlags(uint32_t a, uint32_t b, uint32_t result)
    {
        cpsr.c = CARRY_SUB(a, b);
        cpsr.v = SUB_OVERFLOW(a, b, result);
    }

    bool CondPassed(uint8_t cond);

    bool interruptPending = false;
    bool waitForInterrupt = false;
};