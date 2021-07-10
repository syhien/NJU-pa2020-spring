#include "cpu/instr.h"
/*
Put the implementations of `leave' instructions here.
*/
make_instr_func(leave)
{
    cpu.esp = cpu.ebp;
    OPERAND tmp;
    tmp.data_size = data_size;
    tmp.type = OPR_MEM;
    tmp.sreg = SREG_DS;
    tmp.addr = cpu.esp;
    operand_read(&tmp);
    cpu.ebp = tmp.val;
    cpu.esp += data_size / 8;
    return 1;
}