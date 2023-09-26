/**
 * Additional Xilinx FTM extensions for the public API.
 * see header file for documentation
 */

#include "cs_ftm.h"

#include <assert.h>

#include "csregisters.h"
#include "cs_access_cmnfns.h"

void cs_ftm_set_sync_reload(cs_device_t ftm, unsigned int sync_reload)
{
    assert(12 <= sync_reload && sync_reload <= 0x0fff);

    struct cs_device *d = DEV(ftm);
    assert(d->type == DEV_FTM);

    _cs_set_mask(d, CS_FTM_SYNC_RELOAD, CS_FTM_SYNC_RELOAD_BITMASK, sync_reload);
}

unsigned int cs_ftm_get_sync_reload(cs_device_t ftm)
{
    struct cs_device *d = DEV(ftm);
    assert(d->type == DEV_FTM);

    return _cs_read(d, CS_FTM_SYNC_RELOAD) & CS_FTM_SYNC_RELOAD_BITMASK;
}
