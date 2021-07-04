//
// Created by Frank's Laptop on 2021/7/2.
//

#ifndef RISC_V_SIMULATOR_PREDICTOR_H
#define RISC_V_SIMULATOR_PREDICTOR_H

#include "global.h"
#include <cstring>

class predictor {
public:
    u32 tot, success;

    u8 bht[4096];                     // Branch History Table & Branch Target Buffer
    u32 btb[256];

    predictor() : tot(0), success(0) {
        memset(bht, 0b01u, sizeof(bht)); // default weakly not taken
        memset(btb, 0u, sizeof(btb));
    }


    u32 predictPC(u32 pc, u32 npc) {            // pc & next pc
        if ((pc & 0b1111111u) == 0b1100011u && (bht[pc & 0xFFFu] & 0b10u))
            return btb[pc & 0xFFu];
        else return npc;
    }

    void update(u32 pc, bool jump, u32 jumpAddr, bool scss) {  // change state
        tot++;
        if (scss) success++;

        if (jump && bht[pc & 0xFFFu] < 0b11u) bht[pc & 0xFFFu]++;
        else if (!jump && bht[pc & 0xFFFu] > 0b00u) bht[pc & 0xFFFu]--;
        btb[pc & 0xFFu] = jumpAddr;
    }
};

#endif //RISC_V_SIMULATOR_PREDICTOR_H
