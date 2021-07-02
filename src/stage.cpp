//
// Created by Frank's Laptop on 2021/6/30.
//

#include "stage.h"

//======================== Instruction Fetch ==========================

void stageIF::run() {
    auto &sucBuf = sucBuffer->IF_ID;

    debugPrint("IF: PC = ", pc);

    sucBuf.pc = pc;
    sucBuf.insContent =  // get 4-byte instruction from memory
            (mem->data[pc + 3] << 24u) | (mem->data[pc + 2] << 16u) |
            (mem->data[pc + 1] << 8u) | mem->data[pc];

    pc = pred.predictPC(pc, pc + 4);        // branch prediction
    sucBuf.predictedPc = pc;

    debugPrint("IF: Predicted PC = ", pc);
}


//======================== Instruction Decode =========================

void stageID::run() {
    const auto &preBuf = preBuffer->IF_ID;
    auto &sucBuf = sucBuffer->ID_EX;
    u32 regFlag = 0;

    debugPrint("ID: PC = ", preBuf.pc);
    debugPrint("ID: ppc = ", preBuf.predictedPc);
    debugPrint("ID: ins = ", preBuf.insContent);

    sucBuf.pc = preBuf.pc;
    sucBuf.predictedPc = preBuf.predictedPc;

    INSTRUCTION::decodeIns(preBuf.insContent,   // decode instruction
                           sucBuf.insType,
                           sucBuf.rs1,
                           sucBuf.rs2,
                           sucBuf.rd,
                           sucBuf.imm,
                           regFlag);

    switch (regFlag) {
        case 0:
            sucBuf.rs1 = sucBuf.rv1 = 0u;
            sucBuf.rs2 = sucBuf.rv2 = 0u;
            break;
        case 1:
            sucBuf.rv1 = reg->data[sucBuf.rs1];
            sucBuf.rs2 = sucBuf.rv2 = 0u;
            break;
        case 2:
            sucBuf.rv1 = reg->data[sucBuf.rs1];
            sucBuf.rv2 = reg->data[sucBuf.rs2];
            break;
    }

    if (preBuf.insContent == 0x0FF00513u)
        sucBuf.insType = INSTRUCTION::HALT;
}


//============================= Execute ===============================

void stageEX::run() {
    const auto &preBuf = preBuffer->ID_EX;
    auto &sucBuf = sucBuffer->EX_MEM;
    using INS = INSTRUCTION::insMnemonic;
    using OP = stage::bufferType::EX_MEM_Buffer;

    debugPrint("EX: PC = ", preBuf.pc);
    debugPrint("EX: ppc = ", preBuf.predictedPc);
    debugPrint("EX: insType = ", preBuf.insType);
    debugPrint("EX: rv1 = ", preBuf.rv1);
    debugPrint("EX: rv2 = ", preBuf.rv2);
    debugPrint("EX: rd = ", preBuf.rd);
    debugPrint("EX: imm = ", preBuf.imm);

    sucBuf.predictedPc = preBuf.predictedPc;
    const u32 &pc = sucBuf.pc = preBuf.pc;
    const u32 &insType = preBuf.insType;
    const u32 &rs1 = preBuf.rs1;
    const u32 &rs2 = preBuf.rs2;
    u32 rv1 = preBuf.rv1;
    u32 rv2 = preBuf.rv2;
    const u32 &rd = preBuf.rd;
    const u32 &imm = preBuf.imm;

    // forward
    rv1 = (rs1 == EX_MEM_Buffer->EX_MEM.rd) ? EX_MEM_Buffer->EX_MEM.cr : rv1;
    rv1 = (rs1 == MEM_WB_Buffer->MEM_WB.rd) ? MEM_WB_Buffer->MEM_WB.cr : rv1;
    rv2 = (rs2 == MEM_WB_Buffer->MEM_WB.rd) ? MEM_WB_Buffer->MEM_WB.cr : rv2;
    rv2 = (rs2 == MEM_WB_Buffer->MEM_WB.rd) ? MEM_WB_Buffer->MEM_WB.cr : rv2;

    sucBuf.op = OP::NONE;
    sucBuf.rd = rd;
    sucBuf.mt = rv1 + imm;


    switch (insType) {
        case INS::LUI:      // reg->data[rd] = imm << 12u;
            sucBuf.op = OP::REG;
            sucBuf.cr = imm;
            break;

        case INS::AUIPC:    // reg->data[rd] = pc + (imm << 12u);
            sucBuf.op = OP::REG;
            sucBuf.cr = pc + imm;
            break;

        case INS::JAL:      // reg->data[rd] = pc + 4; jump to (pc+imm)
            sucBuf.op = OP::REG | OP::JUMP;
            sucBuf.cr = pc + 4;
            sucBuf.jd = pc + imm;
            break;

        case INS::JALR:
            sucBuf.op = OP::REG | OP::JUMP;
            sucBuf.cr = pc + 4;
            sucBuf.jd = ((rv1 + imm) & 0xFFFF'FFFE);
            // reg->data[rd] = PC + 4;
            // jump to ((reg->data[rv1] + imm) & 0xFFFF'FFFE)
            break;

        case INS::BEQ:      // jump to (pc+imm)
            sucBuf.op = OP::BRANCH;
            if (rv1 == rv2) sucBuf.op |= OP::BRANCH_TAKEN, sucBuf.jd = pc + imm;
            break;

        case INS::BNE:
            sucBuf.op = OP::BRANCH;
            if (rv1 != rv2) sucBuf.op |= OP::BRANCH_TAKEN, sucBuf.jd = pc + imm;
            break;

        case INS::BLT:
            sucBuf.op = OP::BRANCH;
            if (int(rv1) < int(rv2)) sucBuf.op |= OP::BRANCH_TAKEN, sucBuf.jd = pc + imm;
            break;

        case INS::BGE:
            sucBuf.op = OP::BRANCH;
            if (int(rv1) >= (rv2)) sucBuf.op |= OP::BRANCH_TAKEN, sucBuf.jd = pc + imm;
            break;

        case INS::BLTU:
            sucBuf.op = OP::BRANCH;
            if (rv1 < rv2) sucBuf.op |= OP::BRANCH_TAKEN, sucBuf.jd = pc + imm;
            break;

        case INS::BGEU:
            sucBuf.op = OP::BRANCH;
            if (rv1 >= rv2) sucBuf.op |= OP::BRANCH_TAKEN, sucBuf.jd = pc + imm;
            break;

        case INS::LB:       // reg->data[rd] = signExtend(mem->data[rv1+offset], 7);
            sucBuf.op = OP::MEM_LOAD | OP::MEM_8 | OP::MEM_SIGNED;
            break;

        case INS::LH:
            sucBuf.op = OP::MEM_LOAD | OP::MEM_16 | OP::MEM_SIGNED;
            break;

        case INS::LW:
            sucBuf.op = OP::MEM_LOAD | OP::MEM_32;
            break;

        case INS::LBU:
            sucBuf.op = OP::MEM_LOAD | OP::MEM_8;
            break;

        case INS::LHU:
            sucBuf.op = OP::MEM_LOAD | OP::MEM_16;
            break;

        case INS::SB:
            sucBuf.op = OP::MEM_STORE | OP::MEM_8;
            sucBuf.cr = rv2 & 0xFFu;
            break;

        case INS::SH:
            sucBuf.op = OP::MEM_STORE | OP::MEM_16;
            sucBuf.cr = rv2 & 0xFFFFu;
            break;

        case INS::SW:
            sucBuf.op = OP::MEM_STORE | OP::MEM_32;
            sucBuf.cr = rv2;
            break;

        case INS::ADDI:     // reg->data[rd] = rv1 + imm;
            sucBuf.op = OP::REG;
            sucBuf.cr = rv1 + imm;
            break;

        case INS::SLTI:     // reg->data[rd] = (int(rv1) < int(imm)) ? 1u : 0u;
            sucBuf.op = OP::REG;
            sucBuf.cr = (int(rv1) < int(imm)) ? 1u : 0u;
            break;

        case INS::SLTIU:
            sucBuf.op = OP::REG;
            sucBuf.cr = (rv1 < imm) ? 1u : 0u;
            break;

        case INS::XORI:
            sucBuf.op = OP::REG;
            sucBuf.cr = rv1 xor imm;
            break;

        case INS::ORI:
            sucBuf.op = OP::REG;
            sucBuf.cr = rv1 or imm;
            break;

        case INS::ANDI:
            sucBuf.op = OP::REG;
            sucBuf.cr = rv1 and imm;
            break;

        case INS::SLLI:     // reg->data[rd] = rv1 << (imm & 0b11111u);
            sucBuf.op = OP::REG;
            sucBuf.cr = rv1 << (imm & 0b11111u);
            break;

        case INS::SRLI:
            sucBuf.op = OP::REG;
            sucBuf.cr = rv1 >> (imm & 0b11111u);
            break;

        case INS::SRAI:     // arithmetic right shift, sign bit is filled in the vacated upper bits
            sucBuf.op = OP::REG;
            sucBuf.cr = rv1 >> (imm & 0b11111u);
            HEX::signExtend(sucBuf.cr, 31 - (imm & 0b11111u));
            break;

        case INS::ADD:      // reg->data[rd] = rv1 + rv2;
            sucBuf.op = OP::REG;
            sucBuf.cr = rv1 + rv2;
            break;

        case INS::SUB:
            sucBuf.op = OP::REG;
            sucBuf.cr = rv1 - rv2;
            break;

        case INS::SLL:
            sucBuf.op = OP::REG;
            sucBuf.cr = rv1 << (rv2 & 0b11111u);
            break;

        case INS::SLT:
            sucBuf.op = OP::REG;
            if (rv1 != 0u) sucBuf.cr = (int(rv1) < int(rv2)) ? 1u : 0u;
            else sucBuf.cr = (int(rv2) != 0) ? 1u : 0u; // rv1 == x0
            break;

        case INS::SLTU:
            sucBuf.op = OP::REG;
            if (rv1 != 0u) sucBuf.cr = (rv1 < rv2) ? 1u : 0u;
            else sucBuf.cr = (rv2 != 0) ? 1u : 0u; // rv1 == x0
            break;

        case INS::XOR:
            sucBuf.op = OP::REG;
            sucBuf.cr = rv1 xor rv2;
            break;

        case INS::SRL:
            sucBuf.op = OP::REG;
            sucBuf.cr = rv1 >> (rv2 & 0b11111u);
            break;

        case INS::SRA:
            sucBuf.op = OP::REG;
            sucBuf.cr = rv1 >> (rv2 & 0b11111u);
            HEX::signExtend(sucBuf.cr, 31u - (rv2 & 0b11111u));
            break;

        case INS::OR:
            sucBuf.op = OP::REG;
            sucBuf.cr = rv1 or rv2;
            break;

        case INS::AND:
            sucBuf.op = OP::REG;
            sucBuf.cr = rv1 and rv2;
            break;

        case INS::HALT:
            sucBuf.op = OP::HALT;
    }

    if (sucBuf.op & 0b0000'0100u) // BRANCH or MEM_STORE
        sucBuf.rd = 0;
}


//========================== Memory Access ============================

void stageMEM::run() {
    debugPrint("MEM: stall=", stall);
    debugPrint("MEM: PC =  ", preBuffer->EX_MEM.pc);
    debugPrint("MEM: ppc=  ", preBuffer->EX_MEM.predictedPc);
    debugPrint("MEM: op =  ", preBuffer->EX_MEM.op);
    debugPrint("MEM: cr =  ", preBuffer->EX_MEM.cr);
    debugPrint("MEM: rd =  ", preBuffer->EX_MEM.rd);
    debugPrint("MEM: mt =  ", preBuffer->EX_MEM.mt);
    debugPrint("MEM: jd =  ", preBuffer->EX_MEM.jd);

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

    if (op == MEM_OP::HALT) {  // program end
        stall = 3;
        sucBuf.op = WB_OP::HALT;
        return;
    }

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

    debugPrint("WB: op = ", preBuf.op);
    debugPrint("WB: cr = ", preBuf.cr);
    debugPrint("WB: rd = ", preBuf.rd);

    if (op == WB_OP::HALT) {
        finishFlag = 1;
        return;
    }

    if (op & WB_OP::REG) reg->data[preBuf.rd] = preBuf.cr;
}
