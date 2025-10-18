/**
 * @file ICM_20948.h
 * @author Srikar Bharadwaj R
 *
 * @brief High-level driver for the ICM20948 9-DOF IMU.
 *
 * @details This file contains the device-specific logic, register definitions,
 * and public API for the ICM20948 sensor.
 *
 * @copyright
 * GNU GENERAL PUBLIC LICENSE
 */

#ifndef ICM20948_H__
#define ICM20948_H__

/*********************************************/
/*                Includes                   */
/*********************************************/

#include "sensor_comm.h"

/*********************************************/
/*                Defines                    */
/*********************************************/

#define ICM20948_WHO_AM_I_REG 0x00
#define ICM20948_EXPECTED_WHO_AM_I 0xEA

/*********************************************/
/*              Enumerations                 */
/*********************************************/

/**
 * @brief ICM20948 specific error codes. Can be used to extend HAL errors.
 */
typedef enum
{
    ICM20948_OK = 0,
    ICM20948_ERR_INVALID_PARAM = -1,
    ICM20948_ERR_COMM = -2,
    ICM20948_ERR_DEVICE_NOT_FOUND = -3,
} icm20948_err_t;

/*********************************************/
/*                Structures                 */
/*********************************************/

/**
 * @brief User-facing configuration structure for the ICM20948 driver.
 * This structure is passed by the user to the init function.
 */
typedef struct
{
    platform_hal_bus_cfg_t bus_cfg; ///< Union containing SPI or TWI bus configuration.
    comm_cfg_t comm_cfg;            ///< Communication mode configuration (blocking/non-blocking).
} icm20948_cfg_t;

/**
 * @brief Main device handle for the ICM20948 driver.
 */
typedef struct
{
    sensor_comm_t comm; ///< The generic communication handle for this device instance.
    // Device-specific state variables will be added here.
} icm20948_dev_t;


/*********************************************/
/*           Function Prototypes             */
/*********************************************/

/**
 * @brief Initializes the ICM20948 device driver.
 *
 * @param[out] p_dev Pointer to the device handle structure to be initialized.
 * @param[in]  p_cfg Pointer to the user-defined configuration structure.
 * @param[in]  is_spi True if using SPI, false if using TWI.
 * @return ICM20948_OK on success, otherwise an error code.
 */
icm20948_err_t icm20948_init(icm20948_dev_t *p_dev, const icm20948_cfg_t *p_cfg, bool is_spi);

/**
 * @brief Verifies the device identity by reading the WHO_AM_I register.
 *
 * @param[in] p_dev Pointer to the initialized device handle.
 * @return ICM20948_OK if the device is found, otherwise ICM20948_ERR_DEVICE_NOT_FOUND.
 */
icm20948_err_t icm20948_who_am_i_check(icm20948_dev_t *p_dev);

#endif // ICM20948_H__
