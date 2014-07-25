#ifndef SIMULATOR_GHC_MACHINE_H_
#define SIMULATOR_GHC_MACHINE_H_

#include "ghc_instruction.h"

#include <vector>
#include <string>

class GHCMachineListener {
 public:
  virtual void OnInt(int i, std::vector<unsigned char>& registers) = 0;
};

class GHCMachine {
 public:
  GHCMachine() : registers_(9, 0), memory_(256, 0), listener_(NULL) {}

  void LoadProgram(std::vector<std::string> lines) {
     for (size_t i = 0; i < lines.size(); i++)
       instructions_.push_back(ParseGHCInstruction(lines[i]));
  }

  void Execute() {
    bool halt = false;
    int cycles = 0;
    while (!halt) {
      int pc = registers_[PC];
      GHCInstruction instruction = instructions_[pc];
      switch (instruction.mnemonic) {
        case MOV: Mov(instruction.arguments[0], instruction.arguments[1]); break;
        case INC: Inc(instruction.arguments[0]); break;
        case DEC: Dec(instruction.arguments[0]); break;
        case ADD: Add(instruction.arguments[0], instruction.arguments[1]); break;
        case SUB: Sub(instruction.arguments[0], instruction.arguments[1]); break;
        case MUL: Mul(instruction.arguments[0], instruction.arguments[1]); break;
        case DIV: Div(instruction.arguments[0], instruction.arguments[1]); break;
        case AND: And(instruction.arguments[0], instruction.arguments[1]); break;
        case OR: Or(instruction.arguments[0], instruction.arguments[1]); break;
        case XOR: Xor(instruction.arguments[0], instruction.arguments[1]); break;
        case JLT: Jlt(instruction.arguments[0], instruction.arguments[1], instruction.arguments[2]); break;
        case JEQ: Jeq(instruction.arguments[0], instruction.arguments[1], instruction.arguments[2]); break;
        case JGT: Jgt(instruction.arguments[0], instruction.arguments[1], instruction.arguments[2]); break;
        case INT: Int(instruction.arguments[0]); break;
        case HLT: halt = true; break;
        default: assert(0); break;
      }
      if (registers_[PC] == pc)
        registers_[PC]++;
      if (++cycles >= 1024)
        break;;
    }
    registers_[PC] = 0;
  }

  void SetListener(GHCMachineListener* listener) {
    listener_ = listener;
  }

 private:
  unsigned char Value(GHCArgument argument) {
    if (argument.type == REGISTER)
      return registers_[argument.id];
    if (argument.type == INDIRECT_REGISTER)
      return memory_[registers_[argument.id]];
    if (argument.type == MEMORY)
      return memory_[argument.id];
    return argument.id;
  }

  void SetValue(GHCArgument argument, unsigned char value) {
    if (argument.type == REGISTER)
      registers_[argument.id] = value;
    else if (argument.type == INDIRECT_REGISTER)
      memory_[registers_[argument.id]] = value;
    else if (argument.type == MEMORY)
      memory_[argument.id] = value;
    else
      assert(0);
  }

  void Mov(GHCArgument dest, GHCArgument src) {
    SetValue(dest, Value(src));
  }
  void Inc(GHCArgument dest) {
    SetValue(dest, Value(dest) + 1);
  }
  void Dec(GHCArgument dest) {
    SetValue(dest, Value(dest) - 1);
  }
  void Add(GHCArgument dest, GHCArgument src) {
    SetValue(dest, Value(dest) + Value(src));
  }
  void Sub(GHCArgument dest, GHCArgument src) {
    SetValue(dest, Value(dest) - Value(src));
  }
  void Mul(GHCArgument dest, GHCArgument src) {
    SetValue(dest, Value(dest) * Value(src));
  }
  void Div(GHCArgument dest, GHCArgument src) {
    assert(Value(src) != 0);
    SetValue(dest, Value(dest) / Value(src));
  }
  void And(GHCArgument dest, GHCArgument src) {
    SetValue(dest, Value(dest) & Value(src));
  }
  void Or(GHCArgument dest, GHCArgument src) {
    SetValue(dest, Value(dest) | Value(src));
  }
  void Xor(GHCArgument dest, GHCArgument src) {
    SetValue(dest, Value(dest) ^ Value(src));
  }
  void Jlt(GHCArgument targ, GHCArgument x, GHCArgument y) {
    assert(targ.type == CONSTANT);
    if (Value(x) < Value(y))
      registers_[PC] = targ.id;
  }
  void Jeq(GHCArgument targ, GHCArgument x, GHCArgument y) {
    assert(targ.type == CONSTANT);
    if (Value(x) == Value(y))
      registers_[PC] = targ.id;
  }
  void Jgt(GHCArgument targ, GHCArgument x, GHCArgument y) {
    assert(targ.type == CONSTANT);
    if (Value(x) > Value(y))
      registers_[PC] = targ.id;
  }
  void Int(GHCArgument i) {
    assert(i.type == CONSTANT);
    if (listener_)
      listener_->OnInt(Value(i), registers_);
  }
  std::vector<GHCInstruction> instructions_;
  std::vector<unsigned char> registers_;
  std::vector<unsigned char> memory_;
  GHCMachineListener* listener_;
};

#endif
