/**
 * @file arch_mmc_driver.h
 *
 */
 
#ifndef _ARCH_MMC_DRIVER_
#define _ARCH_MMC_DRIVER_

#include "arch_bd_driver.h"
#include "Mutex.h"

class BDRequest;

class MMCDriver : public BDDriver
{
  public:
    /**
     * Constructor
     *
     */
    MMCDriver();

    /**
     * Destructor
     *
     */
    virtual ~MMCDriver();

    /**
     * adds the given request to a list and checkes the type of the
     * request. If it is a read or write request, it is beeing executed,
     * or the function returns otherwise.
     *
     */
    uint32 addRequest( BDRequest * );

    /**
     * @param 1 sector where it should be started to read
     * @param 2 number of sectors
     * @param 3 buffer where to save all that was read
     *
     */
    int32 readSector( uint32, uint32, void * );

    /**
     * @param 1 sector where it should be started to write
     * @param 2 number of sectors
     * @param 3 buffer, which content should be written to the sectors
     *
     */
    int32 writeSector( uint32, uint32, void * );

    /**
     * @return number of sectors
     *
     */
    uint32 getNumSectors();

    /**
     * @return size of a sector
     *
     */
    uint32 getSectorSize();

    /**
     * handles the active requests
     *
     */
    void serviceIRQ();

    uint32 SPT;
  private:

    /**
     * @param 1 start address
     * @param 2 buffer where to save the block that was read
     *
     */
    int32 readBlock( uint32, void * );

    /**
     * @param 1 start address
     * @param 2 buffer, which content should be written to the bloc
     *
     */
    int32 writeBlock( uint32, void * );
    Mutex lock_;

    uint32 rca_;
    uint32 sector_size_;
    uint32 num_sectors_;
};

#endif
