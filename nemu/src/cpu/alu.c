#include "cpu/cpu.h"

void set_CF_add(uint32_t res, uint32_t src, size_t data_size)
{
    res = sign_ext(res & (0xFFFFFFFF >> (32 - data_size)), data_size);
    src = sign_ext(src & (0xFFFFFFFF >> (32 - data_size)), data_size);
    cpu.eflags.CF = res < src;
}

void set_PF(uint32_t res)
{
    res = res & 0xFF;
    int count = 0;
    while(res)
    {
        if(res & 1)
            count++;
        res >>= 1;
    }
    cpu.eflags.PF = count % 2 == 0;
}

void set_ZF(uint32_t res)
{
    cpu.eflags.ZF = res == 0;
}

void set_SF(uint32_t res, size_t data_size)
{
    res = sign_ext(res & (0xFFFFFFFF >> (32 - data_size)), data_size);
    cpu.eflags.SF = sign(res);
}

void set_OF_add(uint32_t res, uint32_t src, uint32_t dest, size_t data_size)
{
    switch(data_size)
    {
        case 8:
            res = sign_ext(res & 0xFF, 8);
            src = sign_ext(src & 0xFF, 8);
            dest = sign_ext(dest & 0xFF, 8);
            break;
        case 16:
            res = sign_ext(res & 0xFFFF, 16);
            src = sign_ext(src & 0xFFFF, 16);
            dest = sign_ext(dest & 0xFFFF, 16);
            break;
    }
    if(sign(src) == sign(dest))
        cpu.eflags.OF = sign(src) != sign(res);
    else
        cpu.eflags.OF = 0;
}

uint32_t alu_add(uint32_t src, uint32_t dest, size_t data_size)
{
    uint32_t res = src + dest;
    set_CF_add(res, src, data_size);
    set_PF(res);
    set_ZF(res & (0xFFFFFFFF >> (32 - data_size)));
    set_SF(res, data_size);
    set_OF_add(res, src, dest, data_size);
    return res & (0xFFFFFFFF >> (32 - data_size));
}

void set_CF_adc(uint32_t res, uint32_t src, size_t data_size)
{
    res = sign_ext(res & (0xFFFFFFFF >> (32 - data_size)), data_size);
    src = sign_ext(src & (0xFFFFFFFF >> (32 - data_size)), data_size);
    if(cpu.eflags.CF)
        cpu.eflags.CF = res <= src;
    else
        cpu.eflags.CF = res < src;
}

uint32_t alu_adc(uint32_t src, uint32_t dest, size_t data_size)
{
    uint32_t res = src + dest + cpu.eflags.CF;
    set_CF_adc(res, src, data_size);
    set_PF(res);
    set_ZF(res & (0xFFFFFFFF >> (32 - data_size)));
    set_SF(res, data_size);
    set_OF_add(res, src, dest, data_size);
    return res & (0xFFFFFFFF >> (32 - data_size));
}

void set_CF_sub(uint32_t src, uint32_t dest, size_t data_size)
{
    src = sign_ext(src & (0xFFFFFFFF >> (32 - data_size)), data_size);
    dest = sign_ext(dest & (0xFFFFFFFF >> (32 - data_size)), data_size);
    cpu.eflags.CF = dest < src;
}

void set_OF_sub(uint32_t res, uint32_t src, uint32_t dest, size_t data_size)
{
    res = sign_ext(res & (0xFFFFFFFF >> (32 - data_size)), data_size);
    src = sign_ext(src & (0xFFFFFFFF >> (32 - data_size)), data_size);
    dest = sign_ext(dest & (0xFFFFFFFF >> (32 - data_size)), data_size);
    if(sign(src) != sign(dest))
        cpu.eflags.OF = sign(res) != sign(dest);
    else
        cpu.eflags.OF = 0;
}

uint32_t alu_sub(uint32_t src, uint32_t dest, size_t data_size)
{
    uint32_t res = dest - src;
    set_PF(res);
    set_ZF(res & (0xFFFFFFFF >> (32 - data_size)));
    set_CF_sub(src, dest, data_size);
    set_SF(res, data_size);
    set_OF_sub(res, src, dest, data_size);
    return res & (0xFFFFFFFF >> (32 - data_size));
}

void set_CF_sbb(uint32_t src, uint32_t dest, size_t data_size)
{
    src = sign_ext(src & (0xFFFFFFFF >> (32 - data_size)), data_size);
    dest = sign_ext(dest & (0xFFFFFFFF >> (32 - data_size)), data_size);
    if(cpu.eflags.CF)
        cpu.eflags.CF = dest <= src;
    else
        cpu.eflags.CF = dest < src;
}

uint32_t alu_sbb(uint32_t src, uint32_t dest, size_t data_size)
{
    uint32_t res = dest - (src + cpu.eflags.CF);
    set_CF_sbb(src, dest, data_size);
    set_PF(res);
    set_ZF(res & (0xFFFFFFFF >> (32 - data_size)));
    set_SF(res, data_size);
    set_OF_sub(res, src, dest, data_size);
    return res & (0xFFFFFFFF >> (32 - data_size));
}

uint64_t alu_mul(uint32_t src, uint32_t dest, size_t data_size)
{
    uint64_t res = (uint64_t)src * (uint64_t)dest;
    switch(data_size)
    {
        case 8:
            cpu.eflags.CF = cpu.eflags.OF = (res & 0xFF00) != 0;
            break;
        case 16:
            cpu.eflags.CF = cpu.eflags.OF = (res & 0xFFFF0000) != 0;
            break;
        case 32:
            cpu.eflags.CF = cpu.eflags.OF = (res & 0xFFFFFFFF00000000) != 0;
            break;
    }
    return res;
}

int64_t alu_imul(int32_t src, int32_t dest, size_t data_size)
{
    return (int64_t)src * (int64_t)dest;
}

// need to implement alu_mod before testing
uint32_t alu_div(uint64_t src, uint64_t dest, size_t data_size)
{
    if(!src)
        return 0;
    return dest / src;
}

// need to implement alu_imod before testing
int32_t alu_idiv(int64_t src, int64_t dest, size_t data_size)
{
    if(!src)
        return 0;
    return dest / src;
}

uint32_t alu_mod(uint64_t src, uint64_t dest)
{
    return dest % src;
}

int32_t alu_imod(int64_t src, int64_t dest)
{
    return dest % src;
}

uint32_t alu_and(uint32_t src, uint32_t dest, size_t data_size)
{
    uint32_t res = src & dest;
    set_PF(res);
    set_ZF(res & (0xFFFFFFFF >> (32 - data_size)));
    set_SF(res, data_size);
    return res & (0xFFFFFFFF >> (32 - data_size));
}

uint32_t alu_xor(uint32_t src, uint32_t dest, size_t data_size)
{
    uint32_t res = src ^ dest;
    set_PF(res);
    set_ZF(res & (0xFFFFFFFF >> (32 - data_size)));
    set_SF(res, data_size);
    return res & (0xFFFFFFFF >> (32 - data_size));
}

uint32_t alu_or(uint32_t src, uint32_t dest, size_t data_size)
{
    uint32_t res = src | dest;
    set_PF(res);
    set_ZF(res & (0xFFFFFFFF >> (32 - data_size)));
    set_SF(res, data_size);
    return res & (0xFFFFFFFF >> (32 - data_size));
}

uint32_t alu_shl(uint32_t src, uint32_t dest, size_t data_size)
{
    uint32_t res = dest & (0xFFFFFFFF >> (32 - data_size));
    while(src--)
        switch(data_size)
        {
            case 8:
                cpu.eflags.CF = (res & 0x80) == 0x80;
                res <<= 1;
                break;
            case 16:
                cpu.eflags.CF = (res & 0x8000) == 0x8000;
                res <<= 1;
                break;
            case 32:
                cpu.eflags.CF = (res & 0x80000000) == 0x80000000;
                res <<= 1;
                break;
        }
    set_SF(res, data_size);
    set_PF(res);
    set_ZF(res & (0xFFFFFFFF >> (32 - data_size)));
    return res & (0xFFFFFFFF >> (32 - data_size));
}

uint32_t alu_shr(uint32_t src, uint32_t dest, size_t data_size)
{
    uint32_t res = dest & (0xFFFFFFFF >> (32 - data_size));
    while(src--)
        cpu.eflags.CF = res & 1, res >>= 1;
    set_PF(res);
    set_SF(res, data_size);
    set_ZF(res & (0xFFFFFFFF >> (32 - data_size)));
    return res & (0xFFFFFFFF >> (32 - data_size));
}

uint32_t alu_sar(uint32_t src, uint32_t dest, size_t data_size)
{
    uint32_t res = dest & (0xFFFFFFFF >> (32 - data_size));
    if((data_size == 8 && (res & 0x80) == 0x80) || (data_size == 16 && (res & 0x8000) == 0x8000) || (data_size == 32 && (res & 0x80000000) == 0x80000000))
        switch(data_size)
        {
            case 8:
                while(src--)
                    cpu.eflags.CF = res & 1, res >>= 1, res = res ^ 0x80;
                break;
            case 16:
                while(src--)
                    cpu.eflags.CF = res & 1, res >>= 1, res = res ^ 0x8000;
                break;
            case 32:
                while(src--)
                    cpu.eflags.CF = res & 1, res >>= 1, res = res ^ 0x80000000;
                break;
        }
    else
        return alu_shr(src, dest, data_size);
    set_PF(res);
    set_SF(res, data_size);
    set_ZF(res & (0xFFFFFFFF >> (32 - data_size)));
    return res & (0xFFFFFFFF >> (32 - data_size));
}

uint32_t alu_sal(uint32_t src, uint32_t dest, size_t data_size)
{
    return alu_shl(src, dest, data_size);
}
