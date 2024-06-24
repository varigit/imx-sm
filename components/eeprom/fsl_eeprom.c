/*
 * Copyright 2022 NXP
 * Copyright 2024 Variscite
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* Includes */
#include "stdio.h"
#include "fsl_eeprom.h"

/* Local Defines */

/* Local Types */

/* Local Functions */

static uint16_t Eeprom_MaxTransferSize(uint16_t offset, uint16_t len, 
    uint16_t pageSize);

static status_t Eeprom_LPI2C_Receive(LPI2C_Type *base, uint8_t deviceAddress,
    uint32_t subAddress, uint8_t subAddressSize, uint8_t *rxBuff,
    uint16_t rxBuffSize, uint32_t flags);

/* Local Variables */

/* Global Variables */

/*--------------------------------------------------------------------------*/
/* Initialize EEPROM                                                        */
/*--------------------------------------------------------------------------*/
bool Eeprom_Init(const Eeprom_Type *dev)
{
    status_t status;

    status = LPI2C_MasterStart(dev->i2cBase, dev->devAddr, kLPI2C_Write);

    if (status != kStatus_Success)
    {
        return false;
    }

    status = LPI2C_MasterStop(dev->i2cBase);

    return (status == kStatus_Success);
}

/*--------------------------------------------------------------------------*/
/* Read data from EEPROM                                                    */
/*--------------------------------------------------------------------------*/
bool Eeprom_Read(const Eeprom_Type *dev, uint16_t addr, uint8_t *data,
    uint16_t size)
{
    uint16_t devAddr;
    uint16_t readSize;
    status_t status = kStatus_Success;

    if (dev == NULL || data == NULL)
    {
        return false;
    }

    if ((addr + size) > dev->size || size == 0)
    {
        return false;
    }

    while(size > 0)
    {
        devAddr = dev->devAddr;

        if (dev->subAddrMask)
        {
            devAddr |= (addr >> (8 * dev->subAddrSize) & dev->subAddrMask);
        }
        
        /* Up to 2^(address width in bits) */
        readSize = Eeprom_MaxTransferSize(addr, size, 1UL << (dev->subAddrSize * 8));

        status = Eeprom_LPI2C_Receive(dev->i2cBase, devAddr, addr, 
            dev->subAddrSize, data, readSize, kLPI2C_TransferDefaultFlag);
        
        if (status != kStatus_Success)
        {
            break;
        }

        addr += readSize;
        data += readSize;
        size -= readSize;
    }

    return (status == kStatus_Success);
}

bool Eeprom_Dump(const Eeprom_Type *dev, uint16_t addr, uint16_t size)
{
    uint8_t data[16];
    uint16_t i;
    uint16_t j;

    if ((addr + size) > dev->size || size == 0)
    {
        return false;
    }

    for (i = 0; i < size; i += 16)
    {
        if (!Eeprom_Read(dev, addr + i, data, 16))
        {
            return false;
        }

        printf("%04X: ", addr + i);

        for (j = 0; j < 16; j++)
        {
            if (i + j < size)
            {
                printf("%02X ", data[j]);
            }
            else
            {
                printf("   ");
            }
        }

        printf(" ");

        for (j = 0; j < 16; j++)
        {
            if (i + j < size)
            {
                printf("%c", (data[j] >= 0x20 && data[j] < 0x7F) ? data[j] : '.');
            }
        }

        printf("\n");
    }

    return true;
}

/*--------------------------------------------------------------------------*/
/* Calculate max. EEPROM write size                                         */
/*--------------------------------------------------------------------------*/
static uint16_t Eeprom_MaxTransferSize(uint16_t offset, uint16_t len,
    uint16_t pageSize)
{
	int pageOffset = offset & (pageSize - 1);
	int maxlen = pageSize - pageOffset;

	if (len > maxlen)
    {
		len = maxlen;
    }

	return len;
}

/*--------------------------------------------------------------------------*/
/* LPI2C port receive                                                       */
/*--------------------------------------------------------------------------*/
static status_t Eeprom_LPI2C_Receive(LPI2C_Type *base, uint8_t deviceAddress,
    uint32_t subAddress, uint8_t subAddressSize, uint8_t *rxBuff,
    uint16_t rxBuffSize, uint32_t flags)
{
    lpi2c_master_transfer_t xfer;

#ifdef DEBUG
    printf("LPI2C_Receive\n");
    printf("   base: %p\n", base);
    printf("   deviceAddress: %x\n", deviceAddress);
    printf("   subAddress: %x\n", subAddress);
    printf("   subAddressSize: %d\n", subAddressSize);
    printf("   rxBuff: %p\n", rxBuff);
    printf("   rxBuffSize: %d\n", rxBuffSize);
#endif

    xfer.flags          = flags;
    xfer.slaveAddress   = deviceAddress;
    xfer.direction      = kLPI2C_Read;
    xfer.subaddress     = subAddress;
    xfer.subaddressSize = subAddressSize;
    xfer.data           = rxBuff;
    xfer.dataSize       = rxBuffSize;

    return LPI2C_MasterTransferBlocking(base, &xfer);
}
