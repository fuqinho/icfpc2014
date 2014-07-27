
./ghcasm < input > output

It just resolves labels.

JGT FOOBAR,A,B
HLT
FOOBAR:
HLT
==>
0: JGT 2,A,B
1: HLT
2: HLT

or

JGT FOOBAR,A,B
HLT
FOOBAR: HLT

is also allowed.
