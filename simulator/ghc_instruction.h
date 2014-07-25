#ifndef SIMULATOR_GHC_INSTRUCTION_H_
#define SIMULATOR_GHC_INSTRUCTION_H_

#include <vector>
#include <string>
#include <sstream>
#include <cassert>

enum GHCMnemonic {
  MOV,
  INC,
  DEC,
  ADD,
  SUB,
  MUL,
  DIV,
  AND,
  OR,
  XOR,
  JLT,
  JEQ,
  JGT,
  INT,
  HLT
};

enum GHCRegister {
  A = 0,
  B,
  C,
  D,
  E,
  F,
  G,
  H,
  PC
};

enum GHCArgumentType {
  REGISTER,
  INDIRECT_REGISTER,
  CONSTANT,
  MEMORY
};

struct GHCArgument {
  GHCArgumentType type;
  unsigned int id;
};

struct GHCInstruction {
  GHCMnemonic mnemonic;
  std::vector<GHCArgument> arguments;
};

unsigned int ParseArgumentId(std::string str) {
  if (str == "pc") return PC;
  if (str >= "a" && str <= "h") return str[0] - 'a';
  int id;
  std::stringstream ss(str);
  ss >> id;
  return id;
};

GHCArgument ParseGHCArgument(std::string str) {
  GHCArgument argument;
  if (str[0] == '[') {
    if (isdigit(str[1])) {
      argument.type = MEMORY;
      argument.id = ParseArgumentId(str.substr(1, str.size() - 1));
    } else {
      argument.type = INDIRECT_REGISTER;
      argument.id = ParseArgumentId(str.substr(1, str.size() - 1));
    }
  } else {
    if (isdigit(str[0]))
      argument.type = CONSTANT;
    else
      argument.type = REGISTER;
    argument.id = ParseArgumentId(str);
  }
  return argument;
}

#include <iostream>

GHCInstruction ParseGHCInstruction(std::string line) {
  GHCInstruction instruction;

  transform(line.begin(), line.end(), line.begin(), tolower);
  replace(line.begin(), line.end(), ',', ' ');
  std::stringstream ss(line);

  std::string str_mnemonic, tmp;
  ss >> str_mnemonic;
  std::cout << str_mnemonic << std::endl;

  if (str_mnemonic == "mov") {
    instruction.mnemonic = MOV;
    ss >> tmp; instruction.arguments.push_back(ParseGHCArgument(tmp));
    ss >> tmp; instruction.arguments.push_back(ParseGHCArgument(tmp));
  }
  else if (str_mnemonic == "inc") {
    instruction.mnemonic = INC;
    ss >> tmp; instruction.arguments.push_back(ParseGHCArgument(tmp));
  }
  else if (str_mnemonic == "dec") {
    instruction.mnemonic = DEC;
    ss >> tmp; instruction.arguments.push_back(ParseGHCArgument(tmp));
  }
  else if (str_mnemonic == "add") {
    instruction.mnemonic = ADD;
    ss >> tmp; instruction.arguments.push_back(ParseGHCArgument(tmp));
    ss >> tmp; instruction.arguments.push_back(ParseGHCArgument(tmp));
  }
  else if (str_mnemonic == "sub") {
    instruction.mnemonic = SUB;
    ss >> tmp; instruction.arguments.push_back(ParseGHCArgument(tmp));
    ss >> tmp; instruction.arguments.push_back(ParseGHCArgument(tmp));
  }
  else if (str_mnemonic == "mul") {
    instruction.mnemonic = MUL;
    ss >> tmp; instruction.arguments.push_back(ParseGHCArgument(tmp));
    ss >> tmp; instruction.arguments.push_back(ParseGHCArgument(tmp));
  }
  else if (str_mnemonic == "div") {
    instruction.mnemonic = DIV;
    ss >> tmp; instruction.arguments.push_back(ParseGHCArgument(tmp));
    ss >> tmp; instruction.arguments.push_back(ParseGHCArgument(tmp));
  }
  else if (str_mnemonic == "and") {
    instruction.mnemonic = AND;
    ss >> tmp; instruction.arguments.push_back(ParseGHCArgument(tmp));
    ss >> tmp; instruction.arguments.push_back(ParseGHCArgument(tmp));
  }
  else if (str_mnemonic == "or") {
    instruction.mnemonic = OR;
    ss >> tmp; instruction.arguments.push_back(ParseGHCArgument(tmp));
    ss >> tmp; instruction.arguments.push_back(ParseGHCArgument(tmp));
  }
  else if (str_mnemonic == "xor") {
    instruction.mnemonic = XOR;
    ss >> tmp; instruction.arguments.push_back(ParseGHCArgument(tmp));
    ss >> tmp; instruction.arguments.push_back(ParseGHCArgument(tmp));
  }
  else if (str_mnemonic == "jlt") {
    instruction.mnemonic = JLT;
    ss >> tmp; instruction.arguments.push_back(ParseGHCArgument(tmp));
    ss >> tmp; instruction.arguments.push_back(ParseGHCArgument(tmp));
    ss >> tmp; instruction.arguments.push_back(ParseGHCArgument(tmp));
  }
  else if (str_mnemonic == "jeq") {
    instruction.mnemonic = JEQ;
    ss >> tmp; instruction.arguments.push_back(ParseGHCArgument(tmp));
    ss >> tmp; instruction.arguments.push_back(ParseGHCArgument(tmp));
    ss >> tmp; instruction.arguments.push_back(ParseGHCArgument(tmp));
  }
  else if (str_mnemonic == "jgt") {
    instruction.mnemonic = JGT;
    ss >> tmp; instruction.arguments.push_back(ParseGHCArgument(tmp));
    ss >> tmp; instruction.arguments.push_back(ParseGHCArgument(tmp));
    ss >> tmp; instruction.arguments.push_back(ParseGHCArgument(tmp));
  }
  else if (str_mnemonic == "int") {
    instruction.mnemonic = INT;
    ss >> tmp; instruction.arguments.push_back(ParseGHCArgument(tmp));
  }
  else if (str_mnemonic == "hlt") {
    instruction.mnemonic = HLT;
  }
  else {
    assert(0);
  }
  return instruction;
};

#endif
