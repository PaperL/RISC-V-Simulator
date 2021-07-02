//
// Created by Frank's Laptop on 2021/6/29.
//
#ifndef RISC_V_SIMULATOR_CPU
#define RISC_V_SIMULATOR_CPU

#include "stage.h"
#include <iostream>

class cpu {
private:
    RegisterType *reg;      // register
    MemoryType *mem;        // memory
    stage *pipeline[5];     // 5-stage pipeline
    u32 pc, pc_modified;
    u32 stopFlag;
    predictor pred;

public:
    cpu() : reg(new RegisterType), mem(new MemoryType), pc(0), pc_modified(1u), stopFlag(0),
            pipeline{new stageIF(mem, pc, pred),
                     new stageID(reg, stopFlag),
                     new stageEX(),
                     new stageMEM(mem, pc_modified, pred),
                     new stageWB(reg)} {}

    ~cpu() {
        delete reg;
        delete mem;
        for (auto &i:pipeline) delete i;
    }

    void init(std::istream &inputStream) { mem->MemInit(inputStream); }

    void work() {
        while (!stopFlag) {                                             // simulate cpu clock

            for (int i = 0; i < 5 && !stopFlag; ++i) pipeline[i]->run();// each stage works
            if (pc_modified != 1u) pc = pc_modified, pc_modified = 1u;  // 1 for no modified pc
            for (u32 i = 0; i < 4; ++i) {                               // hand over buffer
                pipeline[i + 1]->preBuffer = pipeline[i]->sucBuffer;
                pipeline[i]->sucBuffer->clear();
            }
        }

        std::cout << (reg->data[10] & 0xFFu) << std::endl;              // output answer

        std::cout << "Branch Prediction Succeeded " << pred.success     // output branch prediction result
                  << " Times in all " << pred.tot << " Times." << std::endl;
        std::cout << "Prediction Success Rate: "
                  << double(pred.success) / double(pred.tot) << std::endl;
    }
};

#endif //RISC_V_SIMULATOR_CPU