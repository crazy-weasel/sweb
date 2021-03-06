//----------------------------------------------------------------------
//   $Id: ArchCommon.cpp,v 1.6 2006/01/20 07:20:04 nightcreature Exp $
//----------------------------------------------------------------------
//
//  $Log: ArchCommon.cpp,v $
//  Revision 1.5  2005/09/28 16:35:43  nightcreature
//  main.cpp: added XenConsole (partly implemented but works) to replace TextConsole
//  in xenbuild, first batch of fixes in xen part
//
//  Revision 1.4  2005/09/21 03:33:52  rotho
//  compiles now, but still doesn't work
//
//  Revision 1.3  2005/08/11 16:55:47  nightcreature
//  preview commit only for robert ;-)
//
//  Revision 1.2  2005/08/06 17:38:35  rotho
//  should work ... for now
//
//  Revision 1.1  2005/08/01 08:18:59  nightcreature
//  initial release, partly dummy implementation, needs changes
//
//
//----------------------------------------------------------------------


#include "ArchCommon.h"
#include "paging-definitions.h"
#include "offsets.h"
#include "kprintf.h"
#include "XenConsole.h"

#define MAX_MEMORY_MAPS 10
#define FOUR_ZEROS 0,0,0,0
#define EIGHT_ZEROS FOUR_ZEROS,FOUR_ZEROS
#define SIXTEEN_ZEROS EIGHT_ZEROS,EIGHT_ZEROS
#define TWENTY_ZEROS SIXTEEN_ZEROS,FOUR_ZEROS
#define FOURTY_ZEROS TWENTY_ZEROS,TWENTY_ZEROS
#define MAX_MODULES 1

struct memory_maps
{
  uint32 used;
  pointer start_address;
  pointer end_address;
  uint32 type;
} __attribute__((__packed__)) memory_maps;

static struct memory_maps mem_map_[MAX_MEMORY_MAPS] = {FOURTY_ZEROS};

struct modules_list
{
  uint32 used;
  pointer start_address;
  pointer end_address;
} __attribute__((__packed__)) modules_list;
static struct modules_list modules_list_[MAX_MODULES] = {0,0,0};

extern pointer kernel_end_address_;

extern "C" void initialiseArchCommonMemoryMaps(uint32 nr_pages);
extern "C" void initialiseArchCommonModulesInformation(uint32 nr_modules, pointer start, pointer end);

void initialiseArchCommonMemoryMaps(uint32 nr_pages)
{
   int i = 0;
   mem_map_[0].type = 1;
   mem_map_[0].start_address = 0;
   mem_map_[0].end_address = nr_pages * PAGE_SIZE;
   mem_map_[0].used = 1;
   for(i=1;i < MAX_MEMORY_MAPS; i++)
     mem_map_[i].used = 0;  
}

void initialiseArchCommonModulesInformation(uint32 nr_modules, pointer start, pointer end)
{
  if(nr_modules == 1)
  {
    modules_list_[0].start_address = start;
    modules_list_[0].end_address = end;
    modules_list_[0].used = 1;
  }
}


pointer ArchCommon::getKernelEndAddress()
{
   return kernel_end_address_;
}

pointer ArchCommon::getFreeKernelMemoryStart()
{
   return kernel_end_address_;
}

pointer ArchCommon::getFreeKernelMemoryEnd()
{
   return (pointer)(VIRT_START + 1024U*1024U*4U); //Kernel start +4MB like in x86
}

//no we don't have one at all (at the moment)
uint32 ArchCommon::haveVESAConsole(uint32 is_paging_set_up)
{
  return 0;
}

//no we don't have vesa at all (at the moment)
uint32 ArchCommon::getVESAConsoleHeight()
{
  return 0;
}

//no we don't have vesa at all (at the moment)
uint32 ArchCommon::getVESAConsoleWidth()
{
  return 0;
}

//no we don't have vesa at all (at the moment)
pointer ArchCommon::getVESAConsoleLFBPtr(uint32 is_paging_set_up)
{
  return 0;
}

//TODO: check if it is still possible...or fake it somehow
//this will be done differently... make it 0 at the moment
pointer ArchCommon::getFBPtr(uint32 is_paging_set_up)
{
  return 0;
//   if (is_paging_set_up)
//     return 0xC00B8000;
//   else
//     return 0x000B8000;
}

//no we don't have vesa at all (at the moment)
uint32 ArchCommon::getVESAConsoleBitsPerPixel()
{
  return 0;
}


uint32 ArchCommon::getNumUseableMemoryRegions()
{
  uint32 i;
   for (i=0;i<MAX_MEMORY_MAPS;++i)
   {
     if (!mem_map_[i].used)
       break;
   }
   return i;
//  return 1;
}


uint32 ArchCommon::getUsableMemoryRegion(uint32 region, pointer &start_address, pointer &end_address, uint32 &type)
{
  if (region >= MAX_MEMORY_MAPS)
     return 1;
//   if (region != 0)
//     return 1; 
  start_address = mem_map_[region].start_address;
  end_address = mem_map_[region].end_address;
  type = mem_map_[region].type;  
  return 0;
}

#define MEMCOPY_LARGE_TYPE uint32

void ArchCommon::memcpy(pointer dest, pointer src, size_t size)
{
  MEMCOPY_LARGE_TYPE *s64 = (MEMCOPY_LARGE_TYPE*)src;
  MEMCOPY_LARGE_TYPE *d64 = (MEMCOPY_LARGE_TYPE*)dest;
  
  uint32 i;
  uint32 num_64_bit_copies = size / (sizeof(MEMCOPY_LARGE_TYPE)*8);
  uint32 num_8_bit_copies = size % (sizeof(MEMCOPY_LARGE_TYPE)*8);
  
  for (i=0;i<num_64_bit_copies;++i)
  {
    d64[0] = s64[0];
    d64[1] = s64[1];
    d64[2] = s64[2];
    d64[3] = s64[3];
    d64[4] = s64[4];
    d64[5] = s64[5];
    d64[6] = s64[6];
    d64[7] = s64[7];
    d64 += 8;
    s64 += 8;
  }
  uint8 *s8 = (uint8*)s64;
  uint8 *d8 = (uint8*)d64;
  
  for (i=0;i<num_8_bit_copies;++i)
  {
    *d8 = *s8;
    ++d8;
    ++s8;
  }
}

void ArchCommon::bzero(pointer s, size_t n, uint32 debug)
{
  if (debug) kprintf("Bzero start\n");
  MEMCOPY_LARGE_TYPE *s64 = (MEMCOPY_LARGE_TYPE*)s;
  uint32 num_64_bit_zeros = n / sizeof(MEMCOPY_LARGE_TYPE);
  uint32 num_8_bit_zeros = n % sizeof(MEMCOPY_LARGE_TYPE);
  uint32 i;
  if (debug) kprintf("Bzero next\n");
  for (i=0;i<num_64_bit_zeros;++i)
  {
    *s64 = 0;
    ++s64;
  }
  uint8 *s8 = (uint8*)s64;
  if (debug) kprintf("Bzero middle\n");
  for (i=0;i<num_8_bit_zeros;++i)
  {
    *s8 = 0;
    ++s8;
  }
  if (debug) kprintf("Bzero end\n");
}

uint32 ArchCommon::getNumModules(uint32 is_paging_set_up)
{
  if(modules_list_[0].used)
    return 1;
  return 0;
}

uint32 ArchCommon::getModuleStartAddress(uint32 num, uint32 is_paging_set_up)
{
  return modules_list_[0].start_address;
}

uint32 ArchCommon::getModuleEndAddress(uint32 num, uint32 is_paging_set_up)
{
  return modules_list_[0].end_address;
}


Console* ArchCommon::createConsole(uint32 count)
{
  return new XenConsole(count);
}


void ArchCommon::idle()
{
  __asm__ __volatile__ ( "hlt" );
}
