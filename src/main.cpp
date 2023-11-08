//
// Created by Frank's Laptop on 2021/6/28.
//
#pragma region HEADER

#include "cpu.hpp"
#include <iostream>

#pragma  endregion HEADER

int main() {
    try {
        cpu *RISC_V_Simulator = new cpu();
        RISC_V_Simulator->init(std::cin);
        RISC_V_Simulator->work();
        delete RISC_V_Simulator;
    }
    catch (std::exception &exception) {
        std::cout << "\nCatch Exception: \"" << exception.what() << "\"\n" << std::endl;
        return 1;
    }
    return 0;
}