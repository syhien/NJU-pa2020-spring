#include "cpu/instr.h"
/*
Put the implementations of `call' instructions here.
*/
make_instr_func(call_near)
{
    opr_src.data_size = data_size;
    opr_src.type = OPR_IMM;
    opr_src.addr = cpu.eip + 1;
    operand_read(&opr_src);
    
    opr_dest.data_size = data_size;
    opr_dest.type = OPR_MEM;
    cpu.esp -= data_size / 8;
    opr_dest.addr = cpu.esp;
    opr_dest.val = cpu.eip + 1 + data_size / 8;
    operand_write(&opr_dest);
    
    cpu.eip += 1 + data_size / 8 + sign_ext(opr_src.val, opr_src.data_size);
    return 0;
}

make_instr_func(call_near_indirect)
{
    opr_src.data_size = opr_dest.data_size = data_size;
    int len = 1;
    len += modrm_rm(eip + 1, &opr_src);
    operand_read(&opr_src);

    cpu.esp -= data_size / 8;
    opr_dest.type = OPR_MEM;
    opr_dest.addr = cpu.esp;
    opr_dest.val = cpu.eip + len;
    operand_write(&opr_dest);
    if (data_size == 16)
        cpu.eip = opr_src.val & 0xFFFF;
    else
        cpu.eip = opr_src.val;
    return 0;
}