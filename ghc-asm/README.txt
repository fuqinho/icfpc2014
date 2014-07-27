
./ghcasm < input > output

Resolves labels.

    JGT FOOBAR,A,B
    HLT
  FOOBAR:
    HLT
==>
  JGT 2,A,B
  HLT
  HLT

or

  JGT FOOBAR,A,B
  HLT
  FOOBAR: HLT

is also allowed.


Resolves constants (like direction (UP,RIGHT,DOWN,LEFT) or int number).
See main.cc.