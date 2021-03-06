//
// Created by Frank's Laptop on 2021/7/2.
//

#ifndef RISC_V_SIMULATOR_PREDICTOR_H
#define RISC_V_SIMULATOR_PREDICTOR_H

#include "global.h"
#include <cstring>

#ifdef RISC_V_SIMULATOR_ONE_LEVEL_BRANCH_PREDICTION

class predictor {
public:
    u32 tot, success;

    u8 bht[4096];                               // Branch History Table
    u32 btb[256];                               // Branch Target Buffer

    predictor() : tot(0), success(0) {
        memset(bht, 0b01u, sizeof(bht));    // default weakly not taken
        memset(btb, 0u, sizeof(btb));
    }


#define BHT_VALUE (bht[pc & 0xFFFu])
#define BTB_VALUE (btb[pc & 0xFFu])

    u32 predictPC(u32 pc, u32 npc, u32 ins) {   // pc & next pc
#ifdef RISC_V_SIMULATOR_DEBUG_PREDICTOR
        if ((ins & 0b1111111u) == 0b1100011u) {
            std::cout << "PREDICTION: PC = " << std::hex << pc << ",\tBHT = " << u32(BHT_VALUE);
            if (BHT_VALUE & 0b10u)
                std::cout <<  ",\tTAKEN,  \tBTB = " << u32(BTB_VALUE) << std::endl;
            else std::cout << ",\tNOT_TAKEN" << std::endl;
        }
#endif
        if ((ins & 0b1111111u) == 0b1100011u && (BHT_VALUE & 0b10u))
            return BTB_VALUE;                   // OPCODE is Branch Type & 2-bit Saturating Counter is TAKEN state
        else return npc;
    }

    void update(u32 pc, bool branchTaken, u32 jumpAddr, bool scss) {
        tot++;
        if (scss) success++;

        if (branchTaken) {
            if (BHT_VALUE < 0b11u) BHT_VALUE++;
            BTB_VALUE = jumpAddr;
        }
        else {
            if (BHT_VALUE > 0b00u) BHT_VALUE--;
        }

#ifdef RISC_V_SIMULATOR_DEBUG_PREDICTOR
        std::cout << "UPDATE:     PC = " << std::hex << pc << ",\tBHT = " << u32(BHT_VALUE);
        if (branchTaken) std::cout << ",\tTAKEN,    ";
        else std::cout << ",\tNOT_TAKEN,";
        if (scss) std::cout << "\tRIGHT,\tTARGET_ADDR = " << jumpAddr << std::endl;
        else std::cout << "\tWRONG,\tTARGET_ADDR = " << jumpAddr << std::endl;
#endif
    }
};

#endif //RISC_V_SIMULATOR_ONE_LEVEL_BRANCH_PREDICTION

#ifdef RISC_V_SIMULATOR_TWO_LEVEL_BRANCH_PREDICTION

class predictor {
public:
    u32 tot, success;

    u8 bht[256];                                // Branch History Table
    u8 pht[256][64];                            // Pattern History Table
    u32 btb[256];                               // Branch Target Buffer

    predictor() : tot(0), success(0) {
        memset(bht, 0u, sizeof(bht));
        memset(pht, 0b01u, sizeof(pht));    // default weakly not taken
        memset(btb, 0u, sizeof(btb));
    }

#define BRANCH_FEATURE (pc & 0xFFu)
#define BHT_VALUE (bht[BRANCH_FEATURE])
#define PHT_VALUE (pht[BRANCH_FEATURE][BHT_VALUE])
#define BTB_VALUE (btb[BRANCH_FEATURE])

    u32 predictPC(u32 pc, u32 npc, u32 ins) {
#ifdef RISC_V_SIMULATOR_DEBUG_PREDICTOR
        if ((ins & 0b1111111u) == 0b1100011u) {
            std::cout << "PREDICTION: PC = " << std::hex << pc
                      << ",\tBHT = " << u32(BHT_VALUE)
                      << ",\tPHT = " << u32(PHT_VALUE);
            if (BHT_VALUE & 0b10u)
                std::cout << ",\tTAKEN,  \tBTB = " << u32(BTB_VALUE) << std::endl;
            else std::cout << ",\tNOT_TAKEN" << std::endl;
        }
#endif

        if ((ins & 0b1111111u) == 0b1100011u && (PHT_VALUE & 0b10u))
            return BTB_VALUE;
        else return npc;
    }

    void update(u32 pc, bool branchTaken, u32 jumpAddr, bool scss) {
#ifdef RISC_V_SIMULATOR_DEBUG_PREDICTOR
        std::cout << "UPDATE:     PC = " << std::hex << pc
                  << ",\tBHT = " << u32(BHT_VALUE)
                  << ",\tPHT = " << u32(PHT_VALUE);
        if (branchTaken) std::cout << ",\tTAKEN,    ";
        else std::cout << ",\tNOT_TAKEN,";
        if (scss) std::cout << "\tRIGHT,\tTARGET_ADDR = " << jumpAddr << std::endl;
        else
            std::cout << "\tWRONG,\tTARGET_ADDR = " << jumpAddr
                      << "\tBTB = " << BTB_VALUE << std::endl;
#endif

        tot++;
        if (scss) success++;

        if (branchTaken) {
            if (PHT_VALUE < 0b11u) PHT_VALUE++;
            BHT_VALUE = u8(BHT_VALUE >> 1u) | 0b100000u;
            BTB_VALUE = jumpAddr;
        }
        else {
            if (PHT_VALUE > 0b00u) PHT_VALUE--;
            BHT_VALUE = BHT_VALUE >> 1u;
        }
    }
};

#endif //RISC_V_SIMULATOR_TWO_LEVEL_BRANCH_PREDICTION

#endif //RISC_V_SIMULATOR_PREDICTOR_H
