//
// Created by Frank's Laptop on 2021/6/28.
//
#ifndef RISC_V_SIMULATOR_STAGE
#define RISC_V_SIMULATOR_STAGE

#include "global.h"
#include <cstring>

class stage {
public:
    union bufferType {
        struct IF_ID_Buffer {
            u32 pc = 0;
            u32 insContent = 0;
        } IF_ID;

        struct ID_EX_Buffer {
            u32 insType = INSTRUCTION::NONE;
            u32 pc = 0;
            u32 rd = 0;
            u32 rs1 = 0;
            u32 rs2 = 0;
            u32 imm = 0;
        } ID_EX;

        struct EX_MEM_Buffer {
            // u32 insType = INSTRUCTION::insMnemonic::NONE;
            enum MEM_OP {
                NONE, REG,
                MEM_LOAD_8, MEM_LOAD_16, MEM_LOAD_32,
                MEM_LOAD_8_U, MEM_LOAD_16_U,
                MEM_STORE_8, MEM_STORE_16, MEM_STORE_32,
            };
            u32 pc = 0;
            MEM_OP op = NONE;
            u32 rd = 0;     // register destination
            u32 mt = 0;     // memory target
            u32 cr = 0;     // calculation result
            u32 jd = 0;     // jump destination
        } EX_MEM;

        struct MEM_WB_Buffer {
            u32 rd = 0;
            u32 result = 0;
        } MEM_WB;

        bufferType() { memset(this, 0, sizeof(*this)); }
    };


    bufferType *preBuffer, *sucBuffer;
    RegisterType *reg;
    MemoryType *mem;
    u32 valid;

    stage(RegisterType *globalRegister, MemoryType *globalMemory)
            : reg(globalRegister), mem(globalMemory),
              preBuffer(new bufferType), sucBuffer(new bufferType),
              valid(0) {}

    virtual void run() = 0;
};


//======================== Instruction Fetch ==========================

class stageIF : public stage {
public:
    u32 pc;

    stageIF(RegisterType *globalRegister, MemoryType *globalMemory)
            : stage(globalRegister, globalMemory),
              pc(0) {}

    void run() override;
};


//======================== Instruction Decode =========================

class stageID : public stage {
public:
    u32 pc;

    stageID(RegisterType *globalRegister, MemoryType *globalMemory)
            : stage(globalRegister, globalMemory),
              pc(0) {}

    void run() override;
};


//============================= Execute ===============================

class stageEX : public stage {
public:
    stageEX(RegisterType *globalRegister, MemoryType *globalMemory)
            : stage(globalRegister, globalMemory) {}

    void run() override;
};


//========================== Memory Access ============================

class stageMEM : public stage {
public:
    stageMEM(RegisterType *globalRegister, MemoryType *globalMemory)
            : stage(globalRegister, globalMemory) {}

    void run() override;
};


//=========================== Write Back ==============================

class stageWB : public stage {
public:
    stageWB(RegisterType *globalRegister, MemoryType *globalMemory)
            : stage(globalRegister, globalMemory) {}

    void run() override;
};

#endif //RISC_V_SIMULATOR_STAGE