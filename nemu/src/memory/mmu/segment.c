#include "cpu/cpu.h"
#include "memory/memory.h"

// return the linear address from the virtual address and segment selector
uint32_t segment_translate(uint32_t offset, uint8_t sreg)
{
	/* TODO: perform segment translation from virtual address to linear address
	 * by reading the invisible part of the segment register 'sreg'
	 */
	return cpu.segReg[sreg].base + offset;
}

// load the invisible part of a segment register
void load_sreg(uint8_t sreg)
{
	/* TODO: load the invisibile part of the segment register 'sreg' by reading the GDT.
	 * The visible part of 'sreg' should be assigned by mov or ljmp already.
	 */
/*
    SegDesc* segdesc = (void*)((uint32_t)hw_mem + cpu.gdtr.base + 8 * cpu.segReg[sreg].index);
    uint32_t base = (segdesc->base_31_24 << 24) + (segdesc->base_23_16 << 16) + segdesc->base_15_0;
    assert(base == 0);
    uint32_t limit = (segdesc->limit_19_16 << 16) + segdesc->limit_15_0;
    assert(limit == 0xFFFFF);
    assert(segdesc->granularity == 1);
    cpu.segReg[sreg].base = base;
    cpu.segReg[sreg].limit = limit;
    cpu.segReg[sreg].privilege_level = segdesc->privilege_level;
    cpu.segReg[sreg].type = (segdesc->segment_type << 4) + segdesc->type;
    cpu.segReg[sreg].soft_use = segdesc->soft_use;
*/
    uint32_t pos = cpu.gdtr.base + 8 * cpu.segReg[sreg].index;
    SegDesc desc;
    desc.val[0] = laddr_read(pos, 4);
    desc.val[1] = laddr_read(pos + 4, 4);
    assert(desc.present == 1);
    assert(desc.granularity == 1);
    
    cpu.segReg[sreg].base = desc.base_15_0 + (desc.base_23_16 << 16) + (desc.base_31_24 << 24);
    assert(cpu.segReg[sreg].base == 0);
    cpu.segReg[sreg].limit = desc.limit_15_0 + (desc.limit_19_16 << 16);
    assert(cpu.segReg[sreg].limit == 0xFFFFF);
    cpu.segReg[sreg].type = desc.type + (desc.segment_type << 4);
    cpu.segReg[sreg].privilege_level = desc.privilege_level;
    cpu.segReg[sreg].soft_use = desc.soft_use;
}
