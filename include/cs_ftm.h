/**
 * Additional Xilinx FTM extensions for the public API.
 *
 * Note: most configuration functions are integrated into the regular config functions.
 *       int cs_set_trace_source_id(cs_device_t dev, cs_atid_t id)
 *       cs_atid_t cs_get_trace_source_id(cs_device_t dev)
 *       int cs_trace_enable(cs_device_t dev)
 *       int cs_trace_disable(cs_device_t dev)
 * version 2019-07-08, pisch
 */

#ifndef _included_cs_ftm_h
#define _included_cs_ftm_h

#include "cs_types.h"

/**
 * sync_reload defines after how many FTM packets a fresh ASYNC packet is added
 * to the FTM stream. min: 12; max: 0x0FFF (12 bit value)
 */
void cs_ftm_set_sync_reload(cs_device_t ftm, unsigned int sync_reload);

unsigned int cs_ftm_get_sync_reload(cs_device_t ftm);

#endif
