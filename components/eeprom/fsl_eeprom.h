/*
 * Copyright 2023 NXP
 * Copyright 2024 Variscite
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FSL_EEPROM_H
#define FSL_EEPROM_H

#include "fsl_lpi2c.h"

/*!
 * @addtogroup eeprom
 * @{
 * @file
 * @brief
 *
 * Header file containing the API for a EEPROM.
 */

/******************************************************************************
 * Definitions
 *****************************************************************************/

/*! PCA9451 driver version. */
#define FSL_EEPROM_DRIVER_VERSION (MAKE_VERSION(1, 0, 0))

/*! EEPROM device info. */
typedef struct
{
    LPI2C_Type *i2cBase;  /*!< I2C base address */
    uint8_t devAddr;      /*!< Device I2C address */
    uint32_t size;        /*!< EEPROM size */
    uint32_t pageSize;    /*!< EEPROM page size */
    uint32_t subAddrSize; /*!< Subaddress size */
    uint32_t subAddrMask; /*!< Subaddress mask */
} Eeprom_Type;

/*******************************************************************************
 * API
 ******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * Initialize a EEPROM
 *
 * @param[in]     dev   Device info.
 *
 * @return True if successful.
 */
bool Eeprom_Init(const Eeprom_Type *dev);

/*!
 * Read data from EEPROM
 *
 * @param[in]     dev      Device info.
 * @param[in]     addr     Address to read from.
 * @param[out]    data     Pointer to data buffer.
 * @param[in]     size     Number of bytes to read.
 *
 * @return True if successful.
 */
bool Eeprom_Read(const Eeprom_Type *dev, uint16_t addr, uint8_t *data, uint16_t size);

/*!
 * Dump EEPROM content
 *
 * @param[in]     dev      Device info.
 * @param[in]     addr     Address to start from.
 * @param[in]     size     Number of bytes to dump.
 *
 * @return True if successful.
 */
bool Eeprom_Dump(const Eeprom_Type *dev, uint16_t addr, uint16_t size);

#if defined(__cplusplus)
}
#endif /*_cplusplus*/
/** @} */

#endif /* FSL_EEPROM_H */
