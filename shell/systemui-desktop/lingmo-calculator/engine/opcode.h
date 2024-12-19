#ifndef CORE_OPCODE_H
#define CORE_OPCODE_H

#include <QString>

class Opcode
{
public:

    enum { Nop = 0, Load, Ref, Function, Add, Sub, Neg, Mul, Div, Pow, Fact, Modulo, IntDiv };

    unsigned type;
    unsigned index;

    Opcode(): type(Nop), index(0) {};
    Opcode( unsigned t ): type(t), index(0) {};
    Opcode( unsigned t, unsigned i ): type(t), index(i) {};
};

#endif // CORE_OPCODE_H
