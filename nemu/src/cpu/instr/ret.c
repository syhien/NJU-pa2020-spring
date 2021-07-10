#include "cpu/instr.h"
/*
Put the implementations of `ret' instructions here.
*/
make_instr_func(ret_near)
{
    opr_src.data_size = data_size;
    opr_src.type = OPR_MEM;
    opr_src.addr = cpu.esp;
    operand_read(&opr_src);
    cpu.esp += data_size / 8;
    cpu.eip = opr_src.val;
    return 0;
}

make_instr_func(ret_near_imm16)
{
    opr_src.data_size = data_size;
    opr_src.type = OPR_MEM;
    opr_src.addr = cpu.esp;
    operand_read(&opr_src);
    cpu.esp += data_size / 8;
    opr_dest.type = OPR_IMM;
    opr_dest.addr = cpu.eip + 1;
    opr_dest.data_size = 16;
    operand_read(&opr_dest);
    cpu.esp += opr_dest.val;
    cpu.eip = opr_src.val;
    return 0;
}