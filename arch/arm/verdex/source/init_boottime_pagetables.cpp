/**
 * @file init_boottime_pagetables.cpp
 *
 */

#include "types.h"
#include "boot-time.h"
#include "paging-definitions.h"
#include "offsets.h"
#include "ArchCommon.h"
#include "kprintf.h"
#include "debug_bochs.h"
#include "init_boottime_pagetables.h"

void initialiseBootTimePaging()
{
  page_directory_entry *pde_start = (page_directory_entry*)(((void*)kernel_page_directory_start) - PHYSICAL_TO_VIRTUAL_OFFSET);

  uint32 i;
  // the verdex board has physical ram mapped to 0xA0000000
  uint32 base = 0xA00;
  // clear the page dir
  for (i = 0; i < 4096; ++i)
    *((uint32*)pde_start) = 0;
  // 1 : 1 mapping of the first 8 mbs
  for (i = 0; i < 8; ++i)
    mapPage(pde_start, i, base + i);
  // 1 : 1 mapping of the first 8 mbs of physical ram
  for (i = 0; i < 8; ++i)
    mapPage(pde_start, base + i, base + i);
  // map first 4 mb for kernel
  for (i = 0; i < 4; ++i)
    mapPage(pde_start, 0x800 + i, base + i);
  // 3gb 1:1 mapping
  for (i = 0; i < 1024; ++i)
    mapPage(pde_start, 0xC00 + i, base + i);
  // map devices from 0x81000000 upwards
  mapPage(pde_start,0x860,0x401);  // uart device
  mapPage(pde_start,0x900,0x440);  // lcd controller
  mapPage(pde_start,0x840,0x40D);  // interrupt controller
  mapPage(pde_start,0x830,0x40A);  // timer
  mapPage(pde_start,0x8C0,0x411);  // mmc controller
}

void removeBootTimeIdentMapping()
{
  // we will not remove anything because we need the first 8 mb 1:1 mapped
}
