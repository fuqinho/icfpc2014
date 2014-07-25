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
  Constant,
  Register,
  Memory
};

struct GHCArgument {
  GHCArgumentType type;
  bool as_address;
  unsigned int id;
};

struct GHCInstruction {
  GHCMnemonic mnemonic;
  std::vector<GHCArgument> arguments;
};

