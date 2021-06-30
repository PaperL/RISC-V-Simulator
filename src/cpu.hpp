//
// Created by Frank's Laptop on 2021/6/29.
//
#ifndef RISC_V_SIMULATOR_CPU
#define RISC_V_SIMULATOR_CPU

#include "global.h"
#include "stage.hpp"
#include <iostream>

class cpu {
private:
    RegisterType *reg;      // register
    MemoryType *mem;        // memory
    u32 pc;                 // program counter
    stage *pipeline[5];     // 5-stage pipeline

public:
    cpu() : reg(new RegisterType), mem(new MemoryType), pc(0),
            pipeline{new stageIF(reg, mem, pc), new stageID(reg, mem), new stageEX(reg, mem),
                     new stageMEM(reg, mem), new stageWB(reg, mem)} {

    }

    ~cpu() {
        delete reg;
        delete mem;
        for (auto &i:pipeline) delete i;
    }

    void init(std::istream &inputStream) { mem->MemInit(inputStream); }

    void work() {
        while (true) {                          // 1 clock
            for (auto &i : pipeline) i->run();  // each stage work
            for (u32 i = 0; i < 4; ++i) {       // hand over buffer
                pipeline[i + 1]->preBuffer = pipeline[i]->sucBuffer;
                pipeline[i]->sucBuffer.clear();
            }

            u32 exitFlag = 1;                   // exit when all stages finish
            for (auto &i : pipeline) exitFlag &= i->finishedFlag;
            if (exitFlag)break;
        }

        std::cout << (reg->data[10] & 0b11111111) << std::endl; // output answer
    }
};

#endif //RISC_V_SIMULATOR_CPU