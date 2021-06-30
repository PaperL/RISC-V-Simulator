//
// Created by Frank's Laptop on 2021/6/29.
//
#ifndef RISC_V_SIMULATOR_CPU
#define RISC_V_SIMULATOR_CPU

#include "global.h"
#include "stage.h"
#include <iostream>

class cpu {
private:
    RegisterType *reg;      // register
    MemoryType *mem;        // memory
    stage *pipeline[5];     // 5-stage pipeline

public:
    cpu() : reg(new RegisterType), mem(new MemoryType),
            pipeline{new stageIF(reg, mem), new stageID(reg, mem), new stageEX(reg, mem),
                     new stageMEM(reg, mem), new stageWB(reg, mem)} {

    }

    ~cpu() {
        delete reg;
        delete mem;
        for (auto &i:pipeline) delete i;
    }

    void init(std::istream &inputStream) { mem->MemInit(inputStream); }

    void work() {
        pipeline[0]->valid = 1;                 // begin from IF stage
        while (true) {                          // simulate cpu clock
            for (auto &i : pipeline)            // each stage work
                if (i->valid) i->run();
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