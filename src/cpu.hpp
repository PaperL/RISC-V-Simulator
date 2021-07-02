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
    u32 finishFlag;
    predictor pred;

public:
    cpu() : reg(new RegisterType), mem(new MemoryType), pc(0), pc_modified(-1u), finishFlag(0),
            pipeline{new stageIF(mem, pc, pred),
                     new stageID(reg),
                     nullptr,
                     new stageMEM(mem, pc_modified, pred),
                     new stageWB(reg, finishFlag)} {
        pipeline[2] = new stageEX(pipeline[3]->preBuffer, pipeline[4]->preBuffer);
    }

    ~cpu() {
        delete reg;
        delete mem;
        for (auto &i:pipeline) delete i;
    }

    void init(std::istream &inputStream) { mem->MemInit(inputStream); }

    void work() {
        while (!finishFlag) {                             // simulate cpu clock
            debugPrint("Period Begin");

            reg->data[0] = 0u;
            pc &= 0xFFFFFFFEu;
            pc_modified = -1u;

            for (auto &i : pipeline) {
                i->run();                               // each stage works
                debugPrint(' ');
            }
            debugPrint("Period End\n=================================");

            if (pc_modified != -1u) pc = pc_modified;   // -1 for no modified pc

            for (u32 i = 0; i < 4; ++i) {               // hand over buffer
                *(pipeline[i + 1]->preBuffer) = *(pipeline[i]->sucBuffer);
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