/**
 * @file ICM20948.h
 * @author Srikar Bharadwaj R (ported from SparkFun C++ library)
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
#include "util/ICM_20948_C.h"

/*********************************************/
/*                Defines                    */
/*********************************************/

#define ICM20948_I2C_ADDR 0x69

/*********************************************/
/*              Enumerations                 */
/*********************************************/

/**
 * @brief ICM20948 specific error codes.
 */
typedef enum
{
    ICM20948_OK = 0,
    ICM20948_ERR_INVALID_PARAM = -1,
    ICM20948_ERR_COMM = -2,
    ICM20948_ERR_DEVICE_NOT_FOUND = -3,
    ICM20948_ERR_PLATFORM_FAIL = -4,
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
    sensor_comm_t comm;       ///< The generic communication handle for this device instance.
    ICM_20948_Device_t c_dev; ///< The C backbone device structure from the original library.
    ICM_20948_Serif_t serif;  ///< The serial interface struct to link HAL to the C backbone.
} icm20948_dev_t;

/*********************************************/
/*           Function Prototypes             */
/*********************************************/

/**
 * @brief Initializes the ICM20948 device driver.
 *
 * @param[out] p_dev Pointer to the device handle structure to be initialized.
 * @param[in]  p_cfg Pointer to the user-defined configuration structure.
 * @return ICM20948_OK on success, otherwise an error code.
 */
icm20948_err_t icm20948_init(icm20948_dev_t *p_dev, icm20948_cfg_t *p_cfg);

/**
 * @brief Verifies the device identity by reading the WHO_AM_I register.
 *
 * @param[in] p_dev Pointer to the initialized device handle.
 * @return ICM20948_OK if the device is found, otherwise an error code.
 */
icm20948_err_t icm20948_who_am_i_check(icm20948_dev_t *p_dev);

/**
 * @brief Reads accelerometer, gyroscope, magnetometer and temperature data.
 *
 * @param[in]  p_dev Pointer to the initialized device handle.
 * @param[out] p_agmt Pointer to the structure to store the sensor data.
 * @return ICM20948_OK on success, otherwise an error code.
 */
icm20948_err_t icm20948_get_agmt(icm20948_dev_t *p_dev, ICM_20948_AGMT_t *p_agmt);

/**
 * @brief Performs a software reset of the device.
 *
 * @param[in] p_dev Pointer to the initialized device handle.
 * @return ICM20948_OK on success, otherwise an error code.
 */
icm20948_err_t icm20948_sw_reset(icm20948_dev_t *p_dev);

/**
 * @brief Puts the device into sleep mode or wakes it up.
 *
 * @param[in] p_dev Pointer to the initialized device handle.
 * @param[in] on True to enter sleep mode, false to wake up.
 * @return ICM20948_OK on success, otherwise an error code.
 */
icm20948_err_t icm20948_sleep(icm20948_dev_t *p_dev, bool on);

/**
 * @brief Enables or disables low power mode.
 *
 * @param[in] p_dev Pointer to the initialized device handle.
 * @param[in] on True to enable low power mode, false to disable.
 * @return ICM20948_OK on success, otherwise an error code.
 */
icm20948_err_t icm20948_low_power(icm20948_dev_t *p_dev, bool on);

/**
 * @brief Sets the clock source for the device.
 *
 * @param[in] p_dev Pointer to the initialized device handle.
 * @param[in] source The clock source to select.
 * @return ICM20948_OK on success, otherwise an error code.
 */
icm20948_err_t icm20948_set_clock_source(icm20948_dev_t *p_dev, ICM_20948_PWR_MGMT_1_CLKSEL_e source);

/**
 * @brief Performs a default startup sequence for the ICM20948.
 *
 * @param[in] p_dev Pointer to the initialized device handle.
 * @return ICM20948_OK on success, otherwise an error code.
 */
icm20948_err_t icm20948_startup_default(icm20948_dev_t *p_dev);

/**
 * @brief Initializes the magnetometer.
 *
 * @param[in] p_dev Pointer to the device handle.
 * @param[in] minimal If true, performs a minimal setup (for DMP).
 * @return ICM20948_OK on success, otherwise an error code.
 */
icm20948_err_t icm20948_startup_magnetometer(icm20948_dev_t *p_dev, bool minimal);

/**
 * @brief Checks the WHO_AM_I register of the magnetometer.
 *
 * @param[in] p_dev Pointer to the device handle.
 * @return ICM20948_OK on success, otherwise an error code.
 */
icm20948_err_t icm20948_mag_who_i_am(icm20948_dev_t *p_dev);

/**
 * @brief Resets the magnetometer.
 *
 * @param[in] p_dev Pointer to the device handle.
 * @return ICM20948_OK on success, otherwise an error code.
 */
icm20948_err_t icm20948_reset_mag(icm20948_dev_t *p_dev);

#endif // ICM20948_H__
