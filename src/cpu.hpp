//
// Created by Frank's Laptop on 2021/6/29.
//
#ifndef RISC_V_SIMULATOR_CPU
#define RISC_V_SIMULATOR_CPU

#include "stage.h"
#include <iostream>
#include <iomanip> // std::dec

class cpu {
private:
    RegisterType *reg;      // register
    MemoryType *mem;        // memory
    stage *pipeline[5];     // 5-stage pipeline
    u32 pc, pc_modified, lastPC;
    u32 IF_ID_EX_Stall_Flag, MEM_Stall_Flag, finishFlag;
    predictor pred;

public:
    cpu() : reg(new RegisterType), mem(new MemoryType),
            pc(0), pc_modified(-1u), lastPC(0), IF_ID_EX_Stall_Flag(0), MEM_Stall_Flag(0), finishFlag(0),
            pipeline{new stageIF(mem, pc, pred),
                     new stageID(reg),
                     nullptr,
                     nullptr,
                     new stageWB(reg, finishFlag)} {
        pipeline[3] = new stageMEM(mem, pc_modified, pred, pipeline[4]->preBuffer, MEM_Stall_Flag);
        pipeline[2] = new stageEX(pipeline[3]->preBuffer, pipeline[4]->preBuffer, IF_ID_EX_Stall_Flag); // forward
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
            lastPC = pc;
            IF_ID_EX_Stall_Flag = 0;

            for (auto &i : pipeline) {
                i->run();                               // each stage works
                debugPrint(' ');
            }
            debugPrint("Period End\n=================================");

            if (pc_modified != -1u) pc = pc_modified;   // -1 for no modified pc

            if (!IF_ID_EX_Stall_Flag || MEM_Stall_Flag) {   // ignore stall when jump
                for (u32 i = 0; i < 4; ++i) {               // hand over buffer
                    *(pipeline[i + 1]->preBuffer) = *(pipeline[i]->sucBuffer);
                    pipeline[i]->sucBuffer->clear();
                }
            }
            else { // stall IF,ID,EX stage for 1 period
                debugPrint("IF_ID_EX_Stall_Flag = 1");
                for (u32 i = 2; i < 4; ++i) {
                    *(pipeline[i + 1]->preBuffer) = *(pipeline[i]->sucBuffer);
                    pipeline[i]->sucBuffer->clear();
                }
                pc = lastPC;
            }

            if (MEM_Stall_Flag > 0) MEM_Stall_Flag--;
        }

        debugPrint("HEX Ans = ", reg->data[10]);
        std::cout << std::dec << (reg->data[10] & 0xFFu) << std::endl;              // output answer

        std::cout << "Branch Prediction Succeeded " << pred.success     // output branch prediction result
                  << " Times in all " << pred.tot << " Times." << std::endl;
        std::cout << "Prediction Success Rate: "
                  << double(pred.success) / double(pred.tot) << std::endl;
    }
};

#endif //RISC_V_SIMULATOR_CPU