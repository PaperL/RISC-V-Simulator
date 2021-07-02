//
// Created by Frank's Laptop on 2021/6/30.
//

#include "stage.h"

//======================== Instruction Fetch ==========================

void stageIF::run() {
    auto &sucBuf = sucBuffer->IF_ID;

    sucBuf.pc = pc;
    sucBuf.insContent =  // get 4-byte instruction from memory
            (mem->data[pc + 3] << 24u) | (mem->data[pc + 2] << 16u) |
            (mem->data[pc + 1] << 8u) | mem->data[pc];

    pc = pred.predictPC(pc, pc + 4);        // branch prediction
    sucBuf.predictedPc = pc;
}


//======================== Instruction Decode =========================

void stageID::run() {
    const auto &preBuf = preBuffer->IF_ID;
    auto &sucBuf = sucBuffer->ID_EX;
    u32 regFlag = 0;

    if (preBuf.insContent == 0x0FF00513u) {     // termination instruction
        stopFlag = 1;
        return;
    }

    sucBuf.pc = preBuf.pc;
    sucBuf.predictedPc = preBuf.predictedPc;

    INSTRUCTION::decodeIns(preBuf.insContent,   // decode instruction
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

    sucBuf.predictedPc = preBuf.predictedPc;
    const u32 &pc = sucBuf.pc = preBuf.pc;
    const u32 &insType = preBuf.insType;
    const u32 &rs1 = preBuf.rs1;
    const u32 &rs2 = preBuf.rs2;
    const u32 &rd = preBuf.rd;
    const u32 &imm = preBuf.imm;

    switch (insType) {
        case INS::LUI:      // reg->data[rd] = imm << 12u;
            sucBuf.op = OP::REG;
            sucBuf.rd = rd;
            sucBuf.cr = imm << 12u;
            break;

        case INS::AUIPC:    // reg->data[rd] = pc + (imm << 12u);
            sucBuf.op = OP::REG;
            sucBuf.rd = rd;
            sucBuf.cr = pc + (imm << 12u);
            break;

        case INS::JAL:      // reg->data[rd] = pc + imm + 4; jump to (pc+imm)
            sucBuf.op = OP::REG | OP::JUMP;
            sucBuf.rd = rd;
            sucBuf.jd = pc + imm;
            sucBuf.cr = sucBuf.jd + 4;
            break;

        case INS::JALR:
            sucBuf.op = OP::REG | OP::JUMP;
            sucBuf.rd = rd;
            sucBuf.jd = ((rs1 + imm) & 0xFFFF'FFFE);
            sucBuf.cr = sucBuf.jd + 4;
            // reg->data[rd] = ((reg->data[rs1] + imm) & 0xFFFF'FFFE) + 4;
            // jump to ((reg->data[rs1] + imm) & 0xFFFF'FFFE)
            break;

        case INS::BEQ:      // jump to (pc+imm)
            sucBuf.op = OP::BRANCH;
            if (rs1 == rs2) sucBuf.op |= OP::BRANCH_TAKEN, sucBuf.jd = pc + imm;
            break;

        case INS::BNE:
            sucBuf.op = OP::BRANCH;
            if (rs1 != rs2) sucBuf.op |= OP::BRANCH_TAKEN, sucBuf.jd = pc + imm;
            break;

        case INS::BLT:
            sucBuf.op = OP::BRANCH;
            if (int(rs1) < int(rs2)) sucBuf.op |= OP::BRANCH_TAKEN, sucBuf.jd = pc + imm;
            break;

        case INS::BGE:
            sucBuf.op = OP::BRANCH;
            if (int(rs1) >= (rs2)) sucBuf.op |= OP::BRANCH_TAKEN, sucBuf.jd = pc + imm;
            break;

        case INS::BLTU:
            sucBuf.op = OP::BRANCH;
            if (rs1 < rs2) sucBuf.op |= OP::BRANCH_TAKEN, sucBuf.jd = pc + imm;
            break;

        case INS::BGEU:
            sucBuf.op = OP::BRANCH;
            if (rs1 >= rs2) sucBuf.op |= OP::BRANCH_TAKEN, sucBuf.jd = pc + imm;
            break;

        case INS::LB:       // reg->data[rd] = signExtend(mem->data[rs1], 7);
            sucBuf.op = OP::MEM_LOAD | OP::MEM_8 | OP::MEM_SIGNED;
            sucBuf.rd = rd;
            sucBuf.mt = rs1;
            break;

        case INS::LH:       // reg->data[rd] = signExtend(mem->data[rs1..rs1+1], 7);
            sucBuf.op = OP::MEM_LOAD | OP::MEM_16 | OP::MEM_SIGNED;
            sucBuf.rd = rd;
            sucBuf.mt = rs1;
            break;

        case INS::LW:
            sucBuf.op = OP::MEM_LOAD | OP::MEM_32;
            sucBuf.rd = rd;
            sucBuf.mt = rs1;
            break;

        case INS::LBU:      // reg->data[rd] = mem->data[rs1];
            sucBuf.op = OP::MEM_LOAD | OP::MEM_8;
            sucBuf.rd = rd;
            sucBuf.mt = rs1;
            break;

        case INS::LHU:
            sucBuf.op = OP::MEM_LOAD | OP::MEM_16;
            sucBuf.rd = rd;
            sucBuf.mt = rs1;
            break;

        case INS::SB:       // mem->data[rs1] = rs2 & 0xFFu;
            sucBuf.op = OP::MEM_STORE | OP::MEM_8;
            sucBuf.mt = rs1;
            sucBuf.cr = rs2 & 0xFFu;
            break;

        case INS::SH:
            sucBuf.op = OP::MEM_STORE | OP::MEM_16;
            sucBuf.mt = rs1;
            sucBuf.cr = rs2 & 0xFFFFu;
            break;

        case INS::SW:
            sucBuf.op = OP::MEM_STORE | OP::MEM_32;
            sucBuf.mt = rs1;
            sucBuf.cr = rs2;
            break;

        case INS::ADDI:     // reg->data[rd] = rs1 + imm;
            sucBuf.op = OP::REG;
            sucBuf.rd = rd;
            sucBuf.cr = rs1 + imm;
            break;

        case INS::SLTI:     // reg->data[rd] = (int(rs1) < int(imm)) ? 1u : 0u;
            sucBuf.op = OP::REG;
            sucBuf.rd = rd;
            sucBuf.cr = (int(rs1) < int(imm)) ? 1u : 0u;
            break;

        case INS::SLTIU:
            sucBuf.op = OP::REG;
            sucBuf.rd = rd;
            sucBuf.cr = (rs1 < imm) ? 1u : 0u;
            break;

        case INS::XORI:
            sucBuf.op = OP::REG;
            sucBuf.rd = rd;
            sucBuf.cr = rs1 xor imm;
            break;

        case INS::ORI:
            sucBuf.op = OP::REG;
            sucBuf.rd = rd;
            sucBuf.cr = rs1 or imm;
            break;

        case INS::ANDI:
            sucBuf.op = OP::REG;
            sucBuf.rd = rd;
            sucBuf.cr = rs1 and imm;
            break;

        case INS::SLLI:     // reg->data[rd] = rs1 << (imm & 0b11111u);
            sucBuf.op = OP::REG;
            sucBuf.rd = rd;
            sucBuf.cr = rs1 << (imm & 0b11111u);
            break;

        case INS::SRLI:
            sucBuf.op = OP::REG;
            sucBuf.rd = rd;
            sucBuf.cr = rs1 >> (imm & 0b11111u);
            break;

        case INS::SRAI:     // arithmetic right shift, sign bit is filled in the vacated upper bits
            sucBuf.op = OP::REG;
            sucBuf.rd = rd;
            sucBuf.cr = rs1 >> (imm & 0b11111u);
            HEX::signExtend(sucBuf.cr, 31 - (imm & 0b11111u));
            break;

        case INS::ADD:      // reg->data[rd] = rs1 + rs2;
            sucBuf.op = OP::REG;
            sucBuf.rd = rd;
            sucBuf.cr = rs1 + rs2;
            break;

        case INS::SUB:
            sucBuf.op = OP::REG;
            sucBuf.rd = rd;
            sucBuf.cr = rs1 - rs2;
            break;

        case INS::SLL:
            sucBuf.op = OP::REG;
            sucBuf.rd = rd;
            sucBuf.cr = rs1 << (rs2 & 0b11111u);
            break;

        case INS::SLT:
            sucBuf.op = OP::REG;
            sucBuf.rd = rd;
            if (rs1 != 0u) sucBuf.cr = (int(rs1) < int(rs2)) ? 1u : 0u;
            else sucBuf.cr = (int(rs2) != 0) ? 1u : 0u; // rs1 == x0
            break;

        case INS::SLTU:
            sucBuf.op = OP::REG;
            sucBuf.rd = rd;
            if (rs1 != 0u) sucBuf.cr = (rs1 < rs2) ? 1u : 0u;
            else sucBuf.cr = (rs2 != 0) ? 1u : 0u; // rs1 == x0
            break;

        case INS::XOR:
            sucBuf.op = OP::REG;
            sucBuf.rd = rd;
            sucBuf.cr = rs1 xor rs2;
            break;

        case INS::SRL:
            sucBuf.op = OP::REG;
            sucBuf.rd = rd;
            sucBuf.cr = rs1 >> (rs2 & 0b11111u);
            break;

        case INS::SRA:
            sucBuf.op = OP::REG;
            sucBuf.rd = rd;
            sucBuf.cr = rs1 >> (rs2 & 0b11111u);
            HEX::signExtend(sucBuf.cr, 31u - (rs2 & 0b11111u));
            break;

        case INS::OR:
            sucBuf.op = OP::REG;
            sucBuf.rd = rd;
            sucBuf.cr = rs1 or rs2;
            break;

        case INS::AND:
            sucBuf.op = OP::REG;
            sucBuf.rd = rd;
            sucBuf.cr = rs1 and rs2;
            break;

    }
}


//========================== Memory Access ============================

void stageMEM::run() {
    if (stall > 0) {
        stall--;
        return;
    }
    const auto &preBuf = preBuffer->EX_MEM;
    auto &sucBuf = sucBuffer->MEM_WB;
    using MEM_OP = stage::bufferType::EX_MEM_Buffer;
    using WB_OP = stage::bufferType::MEM_WB_Buffer;
    const u32 &op = preBuf.op;
    const u32 &opType1 = preBuf.op & MEM_OP::TYPE_1;
    const u32 &opType2 = preBuf.op & MEM_OP::TYPE_2;

    sucBuf.op = WB_OP::NONE;

    if (opType1 & MEM_OP::REG) {
        sucBuf.op |= WB_OP::REG;
        sucBuf.rd = preBuf.rd;
        sucBuf.cr = preBuf.cr;
    }
    if (opType1 & MEM_OP::JUMP) {
        pc = preBuf.jd;
        stall = 3;  // wait for new instruction
    }

    if (opType2 == MEM_OP::MEM_LOAD) {
        sucBuf.op |= WB_OP::REG;
        sucBuf.rd = preBuf.rd;
        switch (op & MEM_OP::MEM_LEN) {
            case MEM_OP::MEM_8:
                sucBuf.cr = mem->data[preBuf.mt];
                if (op & MEM_OP::MEM_SIGNED) HEX::signExtend(sucBuf.cr, 7);
                break;

            case MEM_OP::MEM_16:
                sucBuf.cr = (mem->data[preBuf.mt + 1] << 8u) | mem->data[preBuf.mt];
                if (op & MEM_OP::MEM_SIGNED) HEX::signExtend(sucBuf.cr, 15);
                break;

            case MEM_OP::MEM_32:
                sucBuf.cr = (mem->data[preBuf.mt + 3] << 24u) | (mem->data[preBuf.mt + 2] << 16u) |
                            (mem->data[preBuf.mt + 1] << 8u) | mem->data[preBuf.mt];
                break;
        }
    }
    else if (opType2 == MEM_OP::MEM_STORE) {
        switch (op & MEM_OP::MEM_LEN) {
            case MEM_OP::MEM_8:
                mem->data[preBuf.mt] = preBuf.cr;
                break;

            case MEM_OP::MEM_16:
                mem->data[preBuf.mt] = preBuf.cr;
                mem->data[preBuf.mt + 1] = preBuf.cr >> 8u;
                break;

            case MEM_OP::MEM_32:
                mem->data[preBuf.mt] = preBuf.cr;
                mem->data[preBuf.mt + 1] = preBuf.cr >> 8u;
                mem->data[preBuf.mt + 2] = preBuf.cr >> 16u;
                mem->data[preBuf.mt + 3] = preBuf.cr >> 24u;
                break;
        }
    }
    else if (opType2 == MEM_OP::BRANCH) {
        pred.update(preBuf.pc, op & MEM_OP::BRANCH_TAKEN, preBuf.jd);
        if (preBuf.predictedPc != preBuf.jd) {
            pc = preBuf.jd;
            stall = 3;  // invalid wrong branch instruction
        }
    }
}


//=========================== Write Back ==============================

void stageWB::run() {
    const auto &preBuf = preBuffer->MEM_WB;
    using WB_OP = stage::bufferType::MEM_WB_Buffer;
    const u32 &op = preBuf.op;

    if (op & WB_OP::REG) reg->data[preBuf.rd] = preBuf.cr;
}
