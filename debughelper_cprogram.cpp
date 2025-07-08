// debughelper_cprogram.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <iomanip>
#include <string>
#include <unordered_map>

using namespace std;

// 寄存器名称映射表
const unordered_map<int, string> regNames = {
    {0, "zero"}, {1, "ra"}, {2, "sp"}, {3, "gp"}, {4, "tp"},
    {5, "t0"}, {6, "t1"}, {7, "t2"}, {8, "s0"}, {9, "s1"},//{8, "s0/fp"}
    {10, "a0"}, {11, "a1"}, {12, "a2"}, {13, "a3"}, {14, "a4"},
    {15, "a5"}, {16, "a6"}, {17, "a7"}, {18, "s2"}, {19, "s3"},
    {20, "s4"}, {21, "s5"}, {22, "s6"}, {23, "s7"}, {24, "s8"},
    {25, "s9"}, {26, "s10"}, {27, "s11"}, {28, "t3"}, {29, "t4"},
    {30, "t5"}, {31, "t6"}
};

// 指令格式枚举
enum class InstFormat {
    R, I, S, B, U, J
};

// 反汇编函数
string disassemble(uint32_t instruction) {
    uint32_t opcode = instruction & 0x7F;
    uint32_t funct3 = (instruction >> 12) & 0x7;
    uint32_t funct7 = (instruction >> 25) & 0x7F;

    // 提取寄存器和立即数
    uint32_t rd = (instruction >> 7) & 0x1F;
    uint32_t rs1 = (instruction >> 15) & 0x1F;
    uint32_t rs2 = (instruction >> 20) & 0x1F;

    // 立即数解析
    int32_t imm_i = (instruction >> 20) & 0xFFF;
    if (imm_i & 0x800) imm_i |= 0xFFFFF000; // 符号扩展

    int32_t imm_s = ((instruction >> 25) << 5) | ((instruction >> 7) & 0x1F);
    if (imm_s & 0x1000) imm_s |= 0xFFFFE000; // 符号扩展

    int32_t imm_b = ((instruction >> 31) << 12) | ((instruction >> 7) & 0x1) << 11 |
        ((instruction >> 25) & 0x3F) << 5 | ((instruction >> 8) & 0xF) << 1;
    if (imm_b & 0x1000) imm_b |= 0xFFFFE000; // 符号扩展

    int32_t imm_u = (instruction & 0xFFFFF000);

    int32_t imm_j = ((instruction >> 31) << 20) | ((instruction >> 21) & 0x3FF) << 1 |
        ((instruction >> 20) & 0x1) << 11 | ((instruction >> 12) & 0xFF) << 12; 
    if (imm_j & 0x100000) imm_j |= 0xFFE00000; // 符号扩展

    // R-Type指令
    if (opcode == 0x33) {
        if (funct3 == 0x0) {
            if (funct7 == 0x00) return "add " + regNames.at(rd) + ", " + regNames.at(rs1) + ", " + regNames.at(rs2);
            if (funct7 == 0x01) return "mul " + regNames.at(rd) + ", " + regNames.at(rs1) + ", " + regNames.at(rs2);
            if (funct7 == 0x20) return "sub " + regNames.at(rd) + ", " + regNames.at(rs1) + ", " + regNames.at(rs2);
        }
        else if (funct3 == 0x1) {
            if (funct7 == 0x00) return "sll " + regNames.at(rd) + ", " + regNames.at(rs1) + ", " + regNames.at(rs2);
            if (funct7 == 0x01) return "mulh " + regNames.at(rd) + ", " + regNames.at(rs1) + ", " + regNames.at(rs2);
        }
        else if (funct3 == 0x2) {
            if (funct7 == 0x00) return "slt " + regNames.at(rd) + ", " + regNames.at(rs1) + ", " + regNames.at(rs2);
        }
        else if (funct3 == 0x3) {
            if (funct7 == 0x00) return "sltu " + regNames.at(rd) + ", " + regNames.at(rs1) + ", " + regNames.at(rs2);
        }
        else if (funct3 == 0x4) {
            if (funct7 == 0x00) return "xor " + regNames.at(rd) + ", " + regNames.at(rs1) + ", " + regNames.at(rs2);
            if (funct7 == 0x01) return "div " + regNames.at(rd) + ", " + regNames.at(rs1) + ", " + regNames.at(rs2);
        }
        else if (funct3 == 0x5) {
            if (funct7 == 0x00) return "srl " + regNames.at(rd) + ", " + regNames.at(rs1) + ", " + regNames.at(rs2);
            if (funct7 == 0x20) return "sra " + regNames.at(rd) + ", " + regNames.at(rs1) + ", " + regNames.at(rs2);
            if (funct7 == 0x01) return "divu " + regNames.at(rd) + ", " + regNames.at(rs1) + ", " + regNames.at(rs2);
        }
        else if (funct3 == 0x6) {
            if (funct7 == 0x00) return "or " + regNames.at(rd) + ", " + regNames.at(rs1) + ", " + regNames.at(rs2);
            if (funct7 == 0x01) return "rem " + regNames.at(rd) + ", " + regNames.at(rs1) + ", " + regNames.at(rs2);
        }
        else if (funct3 == 0x7) {
            if (funct7 == 0x00) return "and " + regNames.at(rd) + ", " + regNames.at(rs1) + ", " + regNames.at(rs2);
            if (funct7 == 0x01) return "remu " + regNames.at(rd) + ", " + regNames.at(rs1) + ", " + regNames.at(rs2);
        }
    }
    // I-Type指令 - 加载指令
    else if (opcode == 0x03) {
        if (funct3 == 0x0) return "lb " + regNames.at(rd) + ", " + to_string(imm_i) + "(" + regNames.at(rs1) + ")";
        if (funct3 == 0x1) return "lh " + regNames.at(rd) + ", " + to_string(imm_i) + "(" + regNames.at(rs1) + ")";
        if (funct3 == 0x2) return "lw " + regNames.at(rd) + ", " + to_string(imm_i) + "(" + regNames.at(rs1) + ")";
        if (funct3 == 0x4) return "lbu " + regNames.at(rd) + ", " + to_string(imm_i) + "(" + regNames.at(rs1) + ")";
        if (funct3 == 0x5) return "lhu " + regNames.at(rd) + ", " + to_string(imm_i) + "(" + regNames.at(rs1) + ")";
    }
    // I-Type指令 - 立即数操作
    else if (opcode == 0x13) {
        if (funct3 == 0x0) return "addi " + regNames.at(rd) + ", " + regNames.at(rs1) + ", " + to_string(imm_i);
        if (funct3 == 0x1) return "slli " + regNames.at(rd) + ", " + regNames.at(rs1) + ", " + to_string(imm_i & 0x1F);
        if (funct3 == 0x2) return "slti " + regNames.at(rd) + ", " + regNames.at(rs1) + ", " + to_string(imm_i);
        if (funct3 == 0x3) return "sltiu " + regNames.at(rd) + ", " + regNames.at(rs1) + ", " + to_string(imm_i);
        if (funct3 == 0x4) return "xori " + regNames.at(rd) + ", " + regNames.at(rs1) + ", " + to_string(imm_i);
        if (funct3 == 0x5) {
            if ((imm_i & 0x40) == 0) return "srli " + regNames.at(rd) + ", " + regNames.at(rs1) + ", " + to_string(imm_i & 0x1F);
            else return "srai " + regNames.at(rd) + ", " + regNames.at(rs1) + ", " + to_string(imm_i & 0x1F);
        }
        if (funct3 == 0x6) return "ori " + regNames.at(rd) + ", " + regNames.at(rs1) + ", " + to_string(imm_i);
        if (funct3 == 0x7) return "andi " + regNames.at(rd) + ", " + regNames.at(rs1) + ", " + to_string(imm_i);
    }
    // I-Type指令 - 系统指令
    else if (opcode == 0x73) {
        if (funct3 == 0x0) {
            if (imm_i == 0) return "ecall";
            if (imm_i == 1) return "ebreak";
        }
    }
    // S-Type指令 - 存储指令
    else if (opcode == 0x23) {
        if (funct3 == 0x0) return "sb " + regNames.at(rs2) + ", " + to_string(imm_s) + "(" + regNames.at(rs1) + ")";
        if (funct3 == 0x1) return "sh " + regNames.at(rs2) + ", " + to_string(imm_s) + "(" + regNames.at(rs1) + ")";
        if (funct3 == 0x2) return "sw " + regNames.at(rs2) + ", " + to_string(imm_s) + "(" + regNames.at(rs1) + ")";
    }
    // B-Type指令 - 分支指令
    else if (opcode == 0x63) {
        if (funct3 == 0x0) return "beq " + regNames.at(rs1) + ", " + regNames.at(rs2) + ", " + to_string(imm_b);
        if (funct3 == 0x1) return "bne " + regNames.at(rs1) + ", " + regNames.at(rs2) + ", " + to_string(imm_b);
        if (funct3 == 0x4) return "blt " + regNames.at(rs1) + ", " + regNames.at(rs2) + ", " + to_string(imm_b);
        if (funct3 == 0x5) return "bge " + regNames.at(rs1) + ", " + regNames.at(rs2) + ", " + to_string(imm_b);
        if (funct3 == 0x6) return "bltu " + regNames.at(rs1) + ", " + regNames.at(rs2) + ", " + to_string(imm_b);
        if (funct3 == 0x7) return "bgeu " + regNames.at(rs1) + ", " + regNames.at(rs2) + ", " + to_string(imm_b);
    }
    // U-Type指令 - LUI
    else if (opcode == 0x37) {
        return "lui " + regNames.at(rd) + ", " + to_string(imm_u);
    }
    // U-Type指令 - AUIPC
    else if (opcode == 0x17) {
        return "auipc " + regNames.at(rd) + ", " + to_string(imm_u);
    }
    // J-Type指令 - JAL
    else if (opcode == 0x6F) {
        return "jal " + regNames.at(rd) + ", " + to_string(imm_j);
    }
    // I-Type指令 - JALR
    else if (opcode == 0x67) {
        return "jalr " + regNames.at(rd) + ", " + to_string(imm_i) + "(" + regNames.at(rs1) + ")";
    }

    return "unknown instruction: 0x" + to_string(instruction);
}
int main() {
    ifstream fin1("input.txt");
    if (!fin1) return 1;
    ofstream fout1("machinecode.txt");
    vector<string> tokens;
    string line;
    int number = 0;

    // 先按 ';' 分割行,可以改成'\n'
    while (getline(fin1, line, '\n')) {
        stringstream ss(line);
        string token;

        // 再按 ',' 分割每个部分
        while (getline(ss, token, ',')) {
            if (!token.empty()) {  // 避免空字符串
                tokens.push_back("0x"+token);
            }
        }
    }
    
    // 输出结果
    for (size_t i = 0; i < tokens.size(); ++i) {
        fout1 << "0x" << hex << i * 4 << " " << tokens[i] << endl;
    }
    fout1.close();
    vector<uint32_t> machineCodes;
    ifstream fin2("machinecode.txt");
    ofstream fout2("asm.txt");
    while (getline(fin2,line)) {
        stringstream ss(line);
        string token;
        getline(ss, token, ' ');
        getline(ss, token, ' ');
        string hexStr = token;

        // 去除 0x 前缀（如果有）
        if (hexStr.rfind("0x", 0) == 0 || hexStr.rfind("0X", 0) == 0) {
            hexStr = hexStr.substr(2);
        }

        stringstream ss1;
        ss1<< hex << hexStr;  // 设置为十六进制输入
        uint32_t value;
        ss1>> value;
        machineCodes.push_back(value);
    }
    
    fout2 << "RISC-V32 Disassembler" << endl;
    fout2 << "------------------------" << endl;

    for (size_t i = 0; i < machineCodes.size(); i++) {
        uint32_t pc = i * 4;  // 假设每条指令占4字节
        uint32_t instruction = machineCodes[i];

        fout2 << hex << setfill('0') << setw(8) << pc << ": ";
      /*  fout2 << hex << setfill('0') << setw(8) << instruction << "    ";*/
        fout2 << disassemble(instruction) << endl;
    }

    return 0;
}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
