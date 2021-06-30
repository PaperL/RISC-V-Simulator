//
// Created by Frank's Laptop on 2021/6/28.
//
#ifndef RISC_V_SIMULATOR_STAGE
#define RISC_V_SIMULATOR_STAGE

#include "global.h"

class stage {
public:
    u32 finishedFlag = 0;
    BufferType preBuffer, sucBuffer;
    RegisterType *reg;
    MemoryType *mem;
    u32 valid;

    stage(RegisterType *globalRegister, MemoryType *globalMemory)
            : reg(globalRegister), mem(globalMemory), valid(0) {}

    virtual u32 run() = 0;
};



//======================== Instruction Fetch ==========================

class stageIF : public stage {
public:
    u32 &pc;

    stageIF(RegisterType *globalRegister, MemoryType *globalMemory, u32 &globalPC)
            : stage(globalRegister, globalMemory), pc(globalPC) {}

    u32 run() override {
        sucBuffer.data[0] = mem->data[pc] | (mem->data[pc + 1] << 8) |
                            (mem->data[pc + 2] << 16) | (mem->data[pc + 3] << 24);
        pc += 4;
    }
};



//======================== Instruction Decode =========================

class stageID : public stage {
public:
    stageID(RegisterType *globalRegister, MemoryType *globalMemory)
            : stage(globalRegister, globalMemory) {}

    u32 run() override {
        u32 regFlag = 0;
        INSTRUCTION::decodeIns(preBuffer.data[0],
                               sucBuffer.data[0],   // instruction
                               sucBuffer.data[1],   // rs1
                               sucBuffer.data[2],   // rs2
                               sucBuffer.data[3],   // rd
                               sucBuffer.data[4],   // imm
                               regFlag);
        if (regFlag >= 1) {
            sucBuffer.data[3] = reg->data[sucBuffer.data[3]];
            if (regFlag == 2) sucBuffer.data[4] = reg->data[sucBuffer.data[4]];
        }
    }
};



//============================= Execute ===============================

class stageEX : public stage {
public:
    stageEX(RegisterType *globalRegister, MemoryType *globalMemory)
            : stage(globalRegister, globalMemory) {}

    /*
     * preBuffer.data (u32)
     * [0]: instruction (INSTRUCTION::insMnemonic)
     * [1]: rs1
     * [2]: rs2
     * [3]: rd
     * [4]: imm
     */

    u32 run() override {

    }
};



//========================== Memory Access ============================

class stageMEM : public stage {
public:
    stageMEM(RegisterType *globalRegister, MemoryType *globalMemory)
            : stage(globalRegister, globalMemory) {}

    u32 run() override {

    }
};



//=========================== Write Back ==============================

class stageWB : public stage {
public:
    stageWB(RegisterType *globalRegister, MemoryType *globalMemory)
            : stage(globalRegister, globalMemory) {}

    u32 run() override {

    }
};

#endif //RISC_V_SIMULATOR_STAGE