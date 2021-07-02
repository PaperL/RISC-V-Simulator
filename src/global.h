//
// Created by Frank's Laptop on 2021/6/28.
//
#ifndef RISC_V_SIMULATOR_GLOBAL
#define RISC_V_SIMULATOR_GLOBAL

#include <exception>
#include <iostream>
#include <string>
#include <cstring>

using u32 = unsigned int;
using u8 = unsigned char;

namespace HEX {

/*
    const char hexToDec[256] = {
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0,// 10
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 1,// 50
            2, 3, 4, 5, 6, 7, 8, 9, 0, 0,
            0, 0, 0, 0, 0, 10, 11, 12, 13, 14,
            15, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 10, 11, 12,// 100
            13, 14, 15, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0,// 150
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0,// 200
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0,// 250
            0, 0, 0, 0, 0, 0};

    inline u32 char8ToHex(std::string &str, u32 beg = 1) {
        u32 ret = 0;
        for (u32 i = 0; i < 8; i++)
            ret |= hexToDec[str[beg + i]] << ((7 - i) << 2);
        return ret;
    }

    inline u32 char2ToHex(std::string &str) {
        return ((hexToDec[str[0]] << 4) | (hexToDec[str[1]]));
    }
    */

    inline void signExtend(u32 &arg, u32 highBit) {
        if (arg & (1u << highBit))
            arg |= 0b11111111'11111111'11111111'11111111u << (highBit + 1);
    }
}

namespace INSTRUCTION {
    /*
     * In the Book 'The RISC-V Instruction Set Manual'
     * SB is B-type
     * UJ is J-type
     */
    enum insMnemonic {
        NONE,
        HALT,
        LUI,    // U    Load Upper Immediate
        AUIPC,  // U    Add Upper Immediate to PC
        JAL,    // UJ   Jump & Link
        JALR,   // I    Jump & Link Register
        BEQ,    // SB   Branch Equal
        BNE,    // SB   Branch Not Equal
        BLT,    // SB   Branch Less Than
        BGE,    // SB   Branch Greater than or Equal
        BLTU,   // SB   Branch Less than Unsigned
        BGEU,   // SB   Branch Greater than or Equal Unsigned
        LB,     // I    Load Byte
        LH,     // I    Load Halfword
        LW,     // I    Load Word
        LBU,    // I    Load Byte Unsigned
        LHU,    // I    Load Halfword Unsigned
        SB,     // S    Store Byte
        SH,     // S    Store Halfword
        SW,     // S    Store Word
        ADDI,   // I    ADD Immediate
        SLTI,   // I    Set Less than Immediate
        SLTIU,  // I    Set Less than Immediate Unsigned
        XORI,   // I    XOR Immediate
        ORI,    // I    OR Immediate
        ANDI,   // I    AND Immediate
        SLLI,   // I    Shift Left Immediate
        SRLI,   // I    Shift Right Immediate
        SRAI,   // I    Shift Right Arith Immediate
        ADD,    // R    ADD
        SUB,    // R    SUBtract
        SLL,    // R    Shift Left
        SLT,    // R    Set Less than
        SLTU,   // R    Set Less than Unsigned
        XOR,    // R    XOR
        SRL,    // R    Shift Right
        SRA,    // R    Shift Right Arithmetic
        OR,     // R    OR
        AND     // R    AND
    };

    inline void decodeIns(const u32 ins,
                          u32 &instruction, u32 &rs1, u32 &rs2, u32 &rd, u32 &imm,
                          u32 &regFlag) {
        u32 opcode, funct3, funct7;
        opcode = (ins & 0b00000000'00000000'00000000'01111111u);           // Operation Code
        funct3 = (ins & 0b00000000'00000000'01110000'00000000u) >> 12u;    // Function Type 1
        funct7 = (ins & 0b11111110'00000000'00000000'00000000u) >> 25u;    // Function Type 2
        rs1 = (ins & 0b00000000'00001111'10000000'00000000u) >> 15u;    // Register 1
        rs2 = (ins & 0b00000001'11110000'00000000'00000000u) >> 20u;    // Register 2
        rd = (ins & 0b00000000'00000000'00001111'10000000u) >> 7u;     // Memory
        imm = 0;

#define SETMNE(_x) (instruction = ((instruction == insMnemonic::NONE) ? (insMnemonic::_x) : (instruction)))

        instruction = insMnemonic::NONE;
        regFlag = 2;
        switch (opcode) {
            // U
            case 0b0010111u:
                SETMNE(AUIPC);
            case 0b0110111u:
                SETMNE(LUI);
                regFlag = 0;
                imm = (ins & 0b11111111'11111111'11110000'00000000u);   // 31:12
                break;

                // UJ
            case 0b1101111u:
                SETMNE(JAL);
                regFlag = 0;
                imm = (ins & 0b10000000'00000000'00000000'00000000u) >> 11u;   // 20
                imm |= (ins & 0b01111111'11100000'00000000'00000000u) >> 20u;   // 10:1
                imm |= (ins & 0b00000000'00010000'00000000'00000000u) >> 9u;    // 11
                imm |= (ins & 0b00000000'00001111'11110000'00000000u);          // 19:12
                HEX::signExtend(imm, 20);
                break;

                // I
            case 0b0000011u:
                switch (funct3) {
                    case 0b000u:
                        SETMNE(LB);
                    case 0b001u:
                        SETMNE(LH);
                    case 0b010u:
                        SETMNE(LW);
                    case 0b100u:
                        SETMNE(LBU);
                    case 0b101u:
                        SETMNE(LHU);
                }
            case 0b0010011u:
                switch (funct3) {
                    case 0b000u:
                        SETMNE(ADDI);
                    case 0b001u:
                        SETMNE(SLLI);    // funct7 == 0b0000000u
                    case 0b010u:
                        SETMNE(SLTI);
                    case 0b011u:
                        SETMNE(SLTIU);
                    case 0b100u:
                        SETMNE(XORI);
                    case 0b101u:
                        if (funct7 == 0b0000000u) SETMNE(SRLI);
                        if (funct7 == 0b0100000u) SETMNE(SRAI);
                    case 0b110u:
                        SETMNE(ORI);
                    case 0b111u:
                        SETMNE(ANDI);
                }
            case 0b1100111u:
                SETMNE(JALR);
                regFlag = 1;
                imm = (ins & 0b11111111'11110000'00000000'00000000u) >> 20u;   // 11:0
                HEX::signExtend(imm, 11);
                break;

                // S
            case 0b0100011u:
                switch (funct3) {
                    case 0b000u:
                        SETMNE(SB);
                    case 0b001u:
                        SETMNE(SH);
                    case 0b010u:
                        SETMNE(SW);
                }
                imm = (ins & 0b11111110'00000000'00000000'00000000u) >> 20u;   // 11:5
                imm |= (ins & 0b00000000'00000000'00001111'10000000u) >> 7u;    // 4:0
                HEX::signExtend(imm, 11);
                break;

                // R
            case 0b0110011:
                switch (funct3) {
                    case 0b000u:
                        if (funct7 == 0b0000000u) SETMNE(ADD);
                        if (funct7 == 0b0100000u) SETMNE(SUB);
                    case 0b001u:
                        SETMNE(SLL);
                    case 0b010u:
                        SETMNE(SLT);
                    case 0b011u:
                        SETMNE(SLTU);
                    case 0b100u:
                        SETMNE(XOR);
                    case 0b101u:
                        if (funct7 == 0b0000000u) SETMNE(SRL);
                        if (funct7 == 0b0100000u) SETMNE(SRA);
                    case 0b110u:
                        SETMNE(OR);
                    case 0b111u:
                        SETMNE(AND);
                }
                imm = (ins & 0b11111111'11110000'00000000'00000000u) >> 20u;   // 11:0
                HEX::signExtend(imm, 11);

                // SB
            case 0b1100011u:
                switch (funct3) {
                    case 0b000u:
                        SETMNE(BEQ);
                    case 0b001u:
                        SETMNE(BNE);
                    case 0b100u:
                        SETMNE(BLT);
                    case 0b101u:
                        SETMNE(BGE);
                    case 0b110u:
                        SETMNE(BLTU);
                    case 0b111u:
                        SETMNE(BGEU);
                }
                imm = (ins & 0b10000000'00000000'00000000'00000000u) >> 19u;   // 12
                imm = (ins & 0b01111110'00000000'00000000'00000000u) >> 20u;   // 10:5
                imm |= (ins & 0b00000000'00000000'00001111'00000000u) >> 7u;    // 4:0
                imm |= (ins & 0b00000000'00000000'00000000'10000000u) << 4u;    // 11
                HEX::signExtend(imm, 12);
                break;
        }
    }

#undef SETMNE
}

namespace STORAGE {
    template<u32 SIZE = 128, typename dataType = u32>
    struct storageType {
        dataType data[SIZE];

        storageType() : data{0} {}

        void MemInit(std::istream &inputStream) {
            std::string inputString;
            u32 ptr = 0;
            while (!inputStream.fail()) {
                inputStream >> inputString;
                if (inputString[0] == '@') {
                    char *p;
                    ptr = strtoul(inputString.substr(1, 8).c_str(), &p, 16);
                    printf("\n%d!\n", ptr);
                }
                else {
                    char *p;
                    data[ptr] = strtoul(inputString.c_str(), &p, 16);
                    printf("[%s,%d] ", inputString.c_str(), data[ptr]);
                    ptr++;
                }
            }
        }

        storageType<SIZE, dataType> &operator=(const storageType<SIZE, dataType> &other) {
            if (&other != *this) memcpy(data, other.data, sizeof(data));
            return *this;
        }

        void clear() { memset(data, 0, sizeof(data)); }
    };
}

using RegisterType = STORAGE::storageType<32, u32>;
using MemoryType = STORAGE::storageType<1048576, u8>;


void debugPrint(const auto &info) { std::cout << info << std::endl; }

void debugPrintL(const auto &info) { std::cout << info; }

void debugPrint(const auto &... _argList) {
    (debugPrintL(_argList), ...);
    std::cout << std::endl;
}

#endif // RISC_V_SIMULATOR_GLOBAL