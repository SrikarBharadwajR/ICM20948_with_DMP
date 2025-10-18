/**
 * @file platform_hal.h
 * @author Srikar Bharadwaj R
 *
 * @brief Generic Hardware Abstraction Layer (HAL) Interface.
 *
 * @details This header defines a generic interface for hardware communication
 * primitives (SPI, TWI/I2C), basic utilities like delays, and debug logging.
 * To port a driver to a new platform, you only need to provide a
 * C file that implements these function prototypes using the
 * target platform's SDK.
 *
 * @copyright
 * GNU GENERAL PUBLIC LICENSE
 */

#ifndef PLATFORM_HAL_H__
#define PLATFORM_HAL_H__

/*********************************************/
/*                Includes                   */
/*********************************************/

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/*********************************************/
/*                Defines                    */
/*********************************************/

// --- Debug Print Configuration ---
// Define this to enable logging from the HAL layer. Comment it out to disable.
#define PLATFORM_HAL_DEBUG_LOG_ENABLED

#ifdef PLATFORM_HAL_DEBUG_LOG_ENABLED
// In a real application, you would replace printf with your platform's logging utility (e.g., NRF_LOG_INFO)
#define PLATFORM_HAL_LOG(...) printf(__VA_ARGS__)
#else
#define PLATFORM_HAL_LOG(...)
#endif


/*********************************************/
/*              Enumerations                 */
/*********************************************/
/**
 * @brief Generic error codes for the HAL layer.
 */
typedef enum
{
    PLATFORM_HAL_OK = 0,                 ///< Success
    PLATFORM_HAL_ERR_INVALID_PARAM = -1, ///< Invalid parameter
    PLATFORM_HAL_ERR_COMM = -2,          ///< Communication error
    PLATFORM_HAL_ERR_BUSY = -3,          ///< Communication peripheral is busy
    PLATFORM_HAL_ERR_NOT_SUPPORTED = -4, ///< Feature not supported
    PLATFORM_HAL_ERR_TIMEOUT = -5,       ///< Operation timed out
} platform_hal_err_t;

/**
 * @brief Communication mode: blocking or non-blocking.
 */
typedef enum
{
    COMM_MODE_NON_BLOCKING, /**< Non-blocking communication mode */
    COMM_MODE_BLOCKING      /**< Blocking communication mode with timeout */
} comm_mode_t;

/*********************************************/
/*                Unions                     */
/*********************************************/

/**
 * @brief Configuration for communication mode and its parameters.
 */
typedef struct {
    comm_mode_t mode;       /**< The selected communication mode */
    int32_t timeout_ms;     /**< Timeout duration in milliseconds (used in blocking mode) */
} comm_cfg_t;

/**
 * @brief Union for SPI or TWI/I2C bus configuration.
 *
 * Only one member should be used depending on the bus type.
 */
typedef union {
    struct
    {
        void *p_instance; ///< Pointer to SPI peripheral instance
        uint8_t ss_pin;   ///< SPI Slave Select pin
    } spi;

    struct
    {
        void *p_instance; ///< Pointer to TWI/I2C peripheral instance
        uint8_t twi_addr; ///< 7-bit I2C slave address
    } twi;
} platform_hal_bus_cfg_t;

/*********************************************/
/*            Function Pointers              */
/*********************************************/

/**
 * @brief Function pointer for a generic register write function.
 * @param p_bus_cfg Pointer to the platform-specific bus configuration union.
 * @param reg The register address to write to.
 * @param p_data Pointer to the data to write.
 * @param len Number of bytes to write.
 * @param p_comm_cfg Pointer to the communication mode configuration.
 * @return PLATFORM_HAL_OK on success.
 */
typedef platform_hal_err_t (*platform_hal_write_reg_t)(platform_hal_bus_cfg_t *p_bus_cfg, uint8_t reg, const uint8_t *p_data,
                                                       uint32_t len, const comm_cfg_t *p_comm_cfg);

/**
 * @brief Function pointer for a generic register read function.
 * @param p_bus_cfg Pointer to the platform-specific bus configuration union.
 * @param reg The register address to read from.
 * @param p_data Pointer to a buffer to store the read data.
 * @param len Number of bytes to read.
 * @param p_comm_cfg Pointer to the communication mode configuration.
 * @return PLATFORM_HAL_OK on success.
 */
typedef platform_hal_err_t (*platform_hal_read_reg_t)(platform_hal_bus_cfg_t *p_bus_cfg, uint8_t reg, uint8_t *p_data,
                                                      uint32_t len, const comm_cfg_t *p_comm_cfg);

/*********************************************/
/*           Function Prototypes             */
/*********************************************/

/**
 * @brief Platform-specific delay in milliseconds.
 * @param ms Number of milliseconds to delay.
 */
void platform_hal_delay_ms(uint32_t ms);

/**
 * @brief HAL implementation for writing data over SPI.
 */
platform_hal_err_t platform_hal_spi_write(platform_hal_bus_cfg_t *p_bus_cfg, uint8_t reg, const uint8_t *p_data, uint32_t len,
                                          const comm_cfg_t *p_comm_cfg);

/**
 * @brief HAL implementation for reading data over SPI.
 */
platform_hal_err_t platform_hal_spi_read(platform_hal_bus_cfg_t *p_bus_cfg, uint8_t reg, uint8_t *p_data, uint32_t len,
                                         const comm_cfg_t *p_comm_cfg);

/**
 * @brief HAL implementation for writing data over TWI/I2C.
 */
platform_hal_err_t platform_hal_twi_write(platform_hal_bus_cfg_t *p_bus_cfg, uint8_t reg, const uint8_t *p_data, uint32_t len,
                                          const comm_cfg_t *p_comm_cfg);

/**
 * @brief HAL implementation for reading data over TWI/I2C.
 */
platform_hal_err_t platform_hal_twi_read(platform_hal_bus_cfg_t *p_bus_cfg, uint8_t reg, uint8_t *p_data, uint32_t len,
                                         const comm_cfg_t *p_comm_cfg);

#endif // PLATFORM_HAL_H__

