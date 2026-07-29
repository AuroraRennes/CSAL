/*
  Coresight Access Library - API component register access functions

  Copyright (C) ARM Limited, 2013. All rights reserved.

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

  http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
*/

#include "cs_access_cmnfns.h"
#include "cs_reg_access.h"


uint32_t cs_device_read32(cs_device_t dev, unsigned int off)
{
    return _cs_read32(DEV(dev), off);
}

#if CSAL_LEGACY32
uint32_t cs_device_read(cs_device_t dev, unsigned int off)
{
    return cs_device_read32(dev, off);
}
#endif

uint64_t cs_device_read32x2(cs_device_t dev, unsigned int hioff, unsigned int looff)
{
    return ((uint64_t)cs_device_read32(dev, hioff) << 32) |
           cs_device_read32(dev, looff);
}

uint64_t cs_device_read64(cs_device_t dev, unsigned int off)
{
    return _cs_read64(DEV(dev), off);
}

int cs_device_write32(cs_device_t dev, unsigned int off, uint32_t data)
{
    /* Unlock the device if it is locked */
    _cs_unlock(DEV(dev));
    return _cs_write32(DEV(dev), off, data);
}

#if CSAL_LEGACY32
int cs_device_write(cs_device_t dev, unsigned int off, uint32_t data)
{
    return cs_device_write32(dev, off, data);
}
#endif

int cs_device_write64(cs_device_t dev, unsigned int off, uint64_t data)
{
    _cs_unlock(DEV(dev));
    return _cs_write64(DEV(dev), off, data);
}

int cs_device_write32_only(cs_device_t dev, unsigned int off, uint32_t data)
{
    /* Unlock the device if it is locked */
    _cs_unlock(DEV(dev));
    return _cs_write32_wo(DEV(dev), off, data);
}

#if CSAL_LEGACY32
int cs_device_write_only(cs_device_t dev, unsigned int off, uint32_t data)
{
    return cs_device_write32_only(dev, off, data);
}
#endif

int cs_device_write32_masked(cs_device_t dev, unsigned int offset,
                             uint32_t data, uint32_t bitmask)
{
    /* Unlock the device if it is locked */
    _cs_unlock(DEV(dev));
    return _cs_write32_mask(DEV(dev), offset, bitmask, data);
}

#if CSAL_LEGACY32
int cs_device_write_masked(cs_device_t dev, unsigned int offset,
                           uint32_t data, uint32_t bitmask)
{
    return cs_device_write32_masked(dev, offset, data, bitmask);
}
#endif


int cs_device_set32(cs_device_t dev, unsigned int off, uint32_t bits)
{
    _cs_unlock(DEV(dev));
    return _cs_set32(DEV(dev), off, bits);
}

#if CSAL_LEGACY32
int cs_device_set(cs_device_t dev, unsigned int off, uint32_t bits)
{
    return cs_device_set32(dev, off, bits);
}
#endif

int cs_device_clear32(cs_device_t dev, unsigned int off, uint32_t bits)
{
    _cs_unlock(DEV(dev));
    return _cs_clear32(DEV(dev), off, bits);
}

#if CSAL_LEGACY32
int cs_device_clear(cs_device_t dev, unsigned int off, uint32_t bits)
{
    return cs_device_clear32(dev, off, bits);
}
#endif

int cs_device_wait32(cs_device_t dev, unsigned int offset,
                     uint32_t bit_mask, cs_reg_waitbits_op_t operation,
                     uint32_t pattern, uint32_t *p_last_val)
{
    assert((operation >= CS_REG_WAITBITS_ALL_1) && (operation < CS_REG_WAITBITS_END));
    _cs_unlock(DEV(dev));

    return _cs_waitbits32(DEV(dev), offset, bit_mask, operation, pattern,
                          p_last_val);
}

#if CSAL_LEGACY32
int cs_device_wait(cs_device_t dev, unsigned int offset,
                   uint32_t bit_mask, cs_reg_waitbits_op_t operation,
                   uint32_t pattern, uint32_t *p_last_val)
{
    return cs_device_wait32(dev, offset, bit_mask, operation, pattern,
                            p_last_val);
}
#endif

void cs_device_set_wait_repeats(int n_wait_repeat_count)
{
    assert(n_wait_repeat_count >= 1);
    _cs_set_wait_iterations(n_wait_repeat_count);
}

cs_physaddr_t cs_device_address(cs_device_t dev)
{
    struct cs_device *d = DEV(dev);
    return d->phys_addr;
}

unsigned short cs_device_part_number(cs_device_t dev)
{
    struct cs_device *d = DEV(dev);
    return d->part_number;
}


int cs_device_unlock(cs_device_t dev)
{
    return _cs_unlock(DEV(dev));
}


int cs_device_lock(cs_device_t dev)
{
    return _cs_lock(DEV(dev));
}


int cs_device_claim(cs_device_t dev)
{
    return _cs_claim(DEV(dev));
}


int cs_device_unclaim(cs_device_t dev)
{
    return _cs_unclaim(DEV(dev));
}


int cs_device_diag_set(cs_device_t dev, int tracing)
{
#if DIAG
    DEV(dev)->diag_tracing = tracing;
    return 0;
#else
    return -1;
#endif
}

/*
 * Barriers.
 *
 * These ensure completion of a write to a device, and they take a device parameter
 * because there might in principle be different ways of accessing devices.
 * E.g. if a device was accessed through an AXI-AP we might need to push a
 * barrier through the AP.
 *
 * As it is, we assume any MEM-AP is an APB-AP so doesn't need additional
 * action other than the barrier on the write to the MEM-AP itself.
 */

void cs_device_data_barrier(cs_device_t dev)
{
    (void)dev; /* Currently none of the barrier methods are device-specific */
#ifndef USE_DEVMEMD
#if __ARM_ARCH >= 7
    __asm__ __volatile__("dmb sy");
#endif
#else
    /* devmemd can be assumed to have executed sufficient barriers */
#endif
}

void cs_device_instruction_barrier(cs_device_t dev)
{
    (void)dev; /* Currently none of the barrier methods are device-specific */
#ifndef USE_DEVMEMD
#if __ARM_ARCH >= 7
    __asm__ __volatile__("dsb sy");
#endif
#else
    /* devmemd can be assumed to have executed sufficient barriers */
#endif
}

/* end of cs_reg_access.c */
