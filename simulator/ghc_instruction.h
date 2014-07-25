#include <vector>

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

