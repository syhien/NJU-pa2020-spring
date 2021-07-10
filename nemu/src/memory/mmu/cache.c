#include "memory/mmu/cache.h"
#include "memory/memory.h"
#include <time.h>
#include <stdlib.h>

CacheLine cache[1024];
extern uint32_t hw_mem_read(paddr_t paddr, size_t len);
extern void hw_mem_write(paddr_t paddr, size_t len, uint32_t data);

// init the cache
void init_cache()
{
	// implement me in PA 3-1
	srand(time(NULL));
	for (int i = 0; i < 1024; i++)
	    cache[i].valid = false;
}

// write data to cache
void cache_write(paddr_t paddr, size_t len, uint32_t data)
{
	// implement me in PA 3-1
	hw_mem_write(paddr, len, data);
	uint32_t tag = (paddr >> 13) & 0x7ffff;
	uint32_t group = (paddr >> 6) & 0x7f;
	uint32_t offset = paddr & 0x3f;
	int hit;
	for (hit = 0; hit < 8; hit++)
	{
	    if (cache[group * 8 + hit].tag == tag && cache[group * 8 + hit].valid)
	    {
	        if (offset + len <= 64)
	            memcpy(cache[group * 8 + hit].data + offset, &data, len);
            else
            {
                cache_write(paddr, 64 - offset, data);
                cache_write(paddr - offset + 64, len - (64 - offset), data >> (8 * (64 - offset)));
            }
            break;
	    }
	}
}

// read data from cache
uint32_t cache_read(paddr_t paddr, size_t len)
{
	// implement me in PA 3-1
	uint32_t ret = 0;
	uint32_t tag = (paddr >> 13) & 0x7ffff;
	uint32_t group = (paddr >> 6) & 0x7f;
	uint32_t offset = paddr & 0x3f;
	int hit;
	for(hit = 0; hit < 8; hit++)
	{
	    if (cache[group * 8 + hit].tag == tag && cache[group * 8 + hit].valid)
	    {
	        if (offset + len <= 64)
	            memcpy(&ret, cache[group * 8 + hit].data + offset, len);
            else
            {
                uint32_t high;
                memcpy(&ret, cache[group * 8 + hit].data + offset, 64 - offset);
                high = cache_read(paddr - offset + 64, len - (64 - offset)) << (8 * (64 - offset));
                ret |= high;
            }
            break;
	    }
	}
	if (hit == 8)//miss
	{
	    ret = hw_mem_read(paddr, len);
	    for (hit = 0; hit < 8; hit++)
	        if(cache[group * 8 + hit].valid == false)
	        {
	            cache[group * 8 + hit].valid = true;
	            cache[group * 8 + hit].tag = tag;
	            memcpy(cache[group * 8 + hit].data, hw_mem + (paddr - offset), 64);
	            break;
	        }
        if (hit == 8)//miss
        {
            hit = rand() % 8;
            cache[group * 8 + hit].valid = true;
            cache[group * 8 + hit].tag = tag;
            memcpy(cache[group * 8 + hit].data, hw_mem + (paddr - offset), 64);
        }
	}
	return ret;
}

