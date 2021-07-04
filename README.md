# RISC-V Simulator

## 简介

- 上海交通大学，2020级ACM班，PaperL
- 2020-2021学年暑期小学期，PPCA课程项目
- **使用 C++ 模拟 CPU 五级流水**



## 文件结构

- **main.cpp**
  - `main` 函数
- **global.h**
  - namespace `HEX`
    - 符号扩展
  - namespace `INSTRUCTION`
    - 指令类型枚举
    - `decode` 函数
  - namespace `STORAGE`
    - 存储器结构体
  - `debug` 调试信息输出函数

- **cpu.hpp**
  - `cpu` 类
    - 成员变量：寄存器对象、内存对象、各个阶段对象、分支预测器对象、全局变量
    - 成员函数：
      - `init()` 以输入流数据初始化内存信息
      - `work()` 按周期模拟 CPU 运行
- **predictor.hpp**
  - `predictor` 类
    - 成员变量：`bht`、`btb`、总预测次数、预测成功次数
    - 成员函数
      - `predictPC` 函数：根据`branch` 指令 `pc` 提供预测跳转 `pc`
      - `update` 函数：以预测结果更新 `bht`、`btb`、预测次数
- **stage.h** / **stage.cpp**
  - 基类 `stage`
    - 成员变量：`preBuffer`、`sucBuffer`
    - 成员函数：构造函数、`run()`
  - 派生类 `stageIF`、`stageID`、`stageEX`、`stageMEM`、`stageWB`
    - 派生变量：所需的引用。例如寄存器对象、内存对象、回传数据所在缓冲区对象



## 分支预测

- 实现方式
  - 在 `IF (Instruction Fetch)` 阶段进行分支预测，以 `PC (Program Counter)` 作为预测依据
  - 通过大小为 `4096` 的 `BHT (Branch History Table)` 预测是否跳转
  - 通过大小为 `256` 的 `BTB (Branch Target Buffer)` 获取预测跳转地址
- 成功率
  - 待完成