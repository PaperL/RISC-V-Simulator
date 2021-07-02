//
// Created by Frank's Laptop on 2021/6/28.
//
#ifndef RISC_V_SIMULATOR_STAGE
#define RISC_V_SIMULATOR_STAGE

#include "global.h"
#include "predictor.hpp"
#include <cstring>

class stage {
public:
    union bufferType {
        struct IF_ID_Buffer {
            u32 pc = 0;
            u32 predictedPc = 0;
            u32 insContent = 0;
        } IF_ID;

        struct ID_EX_Buffer {
            u32 insType = INSTRUCTION::NONE;
            u32 pc = 0;
            u32 predictedPc = 0;
            u32 rd = 0;
            u32 rs1 = 0;    // NOTE:    Here rs1 & rs2 is not register index
            u32 rs2 = 0;    //          but inside value
            u32 imm = 0;
        } ID_EX;

        struct EX_MEM_Buffer {
            u32 pc = 0;
            u32 predictedPc = 0;
            static const u32 NONE       = 0b0000'0000u;

            static const u32 TYPE_1     = 0b0000'0011u;
            static const u32 REG        = 0b0000'0001u;
            static const u32 JUMP       = 0b0000'0010u;

            static const u32 TYPE_2     = 0b0000'1100u;
            static const u32 BRANCH     = 0b0000'0100u;
            static const u32 MEM_LOAD   = 0b0000'1000u;
            static const u32 MEM_STORE  = 0b0000'1100u;

            static const u32 BRANCH_TAKEN=0b0001'0000u;

            static const u32 MEM_LEN    = 0b0011'0000u;
            static const u32 MEM_8      = 0b0001'0000u;
            static const u32 MEM_16     = 0b0010'0000u;
            static const u32 MEM_32     = 0b0011'0000u;

            static const u32 MEM_SIGNED = 0b0100'0000u;

            u32 op = NONE;
            u32 rd = 0;     // register destination
            u32 mt = 0;     // memory target
            u32 cr = 0;     // calculation result
            u32 jd = 0;     // jump destination
        } EX_MEM;

        struct MEM_WB_Buffer {
            static const u32 NONE = 0b0000u;
            static const u32 REG = 0b0001u;
            u32 op = NONE;
            u32 rd = 0;     // register destination
            u32 cr = 0;     // calculation result
        } MEM_WB;

        bufferType() { clear(); }

        void clear() { memset(this, 0, sizeof(*this)); }
    };


    bufferType *preBuffer, *sucBuffer;

    stage() : preBuffer(new bufferType), sucBuffer(new bufferType) {}

    virtual void run() = 0;
};


//======================== Instruction Fetch ==========================

class stageIF : public stage {
public:
    u32 &pc;
    MemoryType *mem;
    predictor &pred;

    stageIF(MemoryType *globalMemory, u32 &globalPC, predictor &globalPredictor)
            : stage(), mem(globalMemory), pc(globalPC), pred(globalPredictor) {}

    void run() override;
};


//======================== Instruction Decode =========================

class stageID : public stage {
public:
    RegisterType *reg;
    u32 &stopFlag;

    stageID(RegisterType *globalRegister, u32 &stopFlag)
            : stage(), reg(globalRegister), stopFlag(stopFlag) {}

    void run() override;
};


//============================= Execute ===============================

class stageEX : public stage {
public:
    stageEX() : stage() {}

    void run() override;
};


//========================== Memory Access ============================

class stageMEM : public stage {
public:
    MemoryType *mem;
    u32 &pc;
    u32 stall = 0;
    predictor &pred;

    stageMEM(MemoryType *globalMemory, u32 &globalPC, predictor &globalPredictor)
            : stage(), mem(globalMemory), pc(globalPC), pred(globalPredictor) {}

    void run() override;
};


//=========================== Write Back ==============================

class stageWB : public stage {
public:
    RegisterType *reg;

    stageWB(RegisterType *globalRegister)
            : stage(), reg(globalRegister) {}

    void run() override;
};

#endif //RISC_V_SIMULATOR_STAGE