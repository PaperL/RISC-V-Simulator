//
// Created by Frank's Laptop on 2021/6/30.
//

#include "stage.h"



//======================== Instruction Fetch ==========================

void stageIF::run() {
    auto &sucBuf = sucBuffer->IF_ID;

    sucBuf.pc = pc;
    sucBuf.insContent =  // get 4-byte instruction from memory
            mem->data[pc] | (mem->data[pc + 1] << 8) |
            (mem->data[pc + 2] << 16) | (mem->data[pc + 3] << 24);
    pc += 4;
}


//======================== Instruction Decode =========================

void stageID::run() {
    const auto &preBuf = preBuffer->IF_ID;
    auto &sucBuf = sucBuffer->ID_EX;
    u32 regFlag = 0;

    sucBuf.pc = preBuf.pc;
    INSTRUCTION::decodeIns(preBuf.insContent,
                           sucBuf.insType,
                           sucBuf.rs1,
                           sucBuf.rs2,
                           sucBuf.rd,
                           sucBuf.imm,
                           regFlag);
    if (regFlag >= 1) {
        sucBuf.rs1 = reg->data[sucBuf.rs1];
        if (regFlag == 2)sucBuf.rs2 = reg->data[sucBuf.rs2];
    }
}


//============================= Execute ===============================

void stageEX::run() {
    const auto &preBuf = preBuffer->ID_EX;
    auto &sucBuf = sucBuffer->EX_MEM;
    using INS = INSTRUCTION::insMnemonic;
    using OP = stage::bufferType::EX_MEM_Buffer;

    const u32 &pc = sucBuf.pc = preBuf.pc;
    const u32 &insType = preBuf.insType;
    const u32 &rs1 = preBuf.rs1;
    const u32 &rs2 = preBuf.rs2;
    const u32 &rd = sucBuf.rd = preBuf.rd;
    const u32 &imm = preBuf.imm;

    switch (insType) {
        case INS::LUI:
            sucBuf.op = OP::REG;
            sucBuf.rd = rd;
            sucBuf.rs = imm << 12u;
            // reg->data[rd] = imm << 12u;
            break;

        case INS::AUIPC:
            sucBuf.op = OP::REG;
            sucBuf.rd = rd;
            sucBuf.rs = pc + (imm << 12u);
            // reg->data[rd] = pc + (imm << 12u);
            break;

        case INS::JAL:
            sucBuf.op = OP::REG;
            sucBuf.rd = rd;
            sucBuf.rs = pc + imm + 4;
            // reg->data[rd] = pc;
            // jump to (pc+imm)
            break;

        case INS::JALR:
            sucBuf.op = OP::REG;
            sucBuf.rd = rd;
            sucBuf.rs = (reg->data[rs1] & 0xFFFF'FFFE) + 4;
            // reg->data[rd] = pc;
            // jump to ((reg->data[rs1] + imm) & 0xFFFF'FFFE)
            break;

        case INS::BEQ:
            if (reg->data[rs1] == reg->data[rs2])
                sucBuf.op = OP::JUMP, sucBuf.rs = pc + imm;
                // jump to (pc+imm)
            else sucBuf.op = OP::NONE;
            break;

        case INS::BNE:
            if (reg->data[rs1] != reg->data[rs2])
                sucBuf.op = OP::JUMP, sucBuf.rs = pc + imm;
            else sucBuf.op = OP::NONE;
            break;

        case INS::BLT:
            if (int(reg->data[rs1]) < int(reg->data[rs2]))
                sucBuf.op = OP::JUMP, sucBuf.rs = pc + imm;
            else sucBuf.op = OP::NONE;
            break;

        case INS::BGE:
            if (int(reg->data[rs1]) >= (reg->data[rs2]))
                sucBuf.op = OP::JUMP, sucBuf.rs = pc + imm;
            else sucBuf.op = OP::NONE;
            break;

        case INS::BLTU:
            if (reg->data[rs1] < reg->data[rs2])
                sucBuf.op = OP::JUMP, sucBuf.rs = pc + imm;
            else sucBuf.op = OP::NONE;
            break;

        case INS::BGEU:
            if (reg->data[rs1] >= reg->data[rs2])
                sucBuf.op = OP::JUMP, sucBuf.rs = pc + imm;
            else sucBuf.op = OP::NONE;
            break;

        case INS::LB:
            sucBuf.op = OP::MEM_LOAD_8;
            sucBuf.rd = rd;
            sucBuf.rs = rs1;
            // reg->data[rd] = INSTRUCTION::signExtend(mem->data[rs1], 7);
            break;

        case INS::LH:
            sucBuf.op = OP::MEM_LOAD_16;
            sucBuf.rd = rd;
            sucBuf.rs = rs1;
            // reg->data[rd] = INSTRUCTION::signExtend(mem->data[rs1..rs1+1], 15);
            break;

        case INS::LW:
            sucBuf.op = OP::MEM_LOAD_32;
            sucBuf.rd = rd;
            sucBuf.rs = rs1;
            // reg->data[rd] = INSTRUCTION::signExtend(mem->data[rs1..rs1+3], 31);
            break;

        case INS::LBU:
            sucBuf.op = OP::MEM_LOAD_8_U;
            sucBuf.rd = rd;
            sucBuf.rs = rs1;
            // reg->data[rd] = mem->data[rs1];

        case INS::LHU:
            sucBuf.op = OP::MEM_LOAD_16_U;
            sucBuf.rd = rd;
            sucBuf.rs = rs1;
            // reg->data[rd] = mem->data[rs1..rs1+1];

        case INS::SB:
            sucBuf.op = OP::MEM_STORE_8;
            sucBuf.rd = rs1;
            sucBuf.rs = rs2 & 0xFFu;
            // mem->data[rs1] = rs2 & 0xFFu;

        case INS::SH:
            sucBuf.op = OP::MEM_STORE_16;
            sucBuf.rd = rs1;
            sucBuf.rs = rs2 & 0xFFFFu;
            // mem->data[rs1] = rs2 & 0xFFFFu;

        case INS::SW:
            sucBuf.op = OP::MEM_STORE_32;
            sucBuf.rd = rs1;
            sucBuf.rs = rs2;
            // mem->data[rs1] = rs2;

        case INS::ADDI:
            sucBuf.op = OP::REG;
            sucBuf.rd = rd;
            sucBuf.rs = rs1 + imm;

    }

}


//========================== Memory Access ============================

void stageMEM::run() {
    return 0;
}


//=========================== Write Back ==============================

void stageWB::run() {
    return 0;
}
