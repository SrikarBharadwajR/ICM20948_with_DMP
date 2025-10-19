/**
 * @file platform_hal_nrf52.h
 * @author Srikar Bharadwaj R
 *
 * @brief Generic Hardware Abstraction Layer (HAL) Interface for nRF52.
 *
 * @details This header defines a generic interface for hardware communication
 * primitives (SPI, TWI/I2C), basic utilities like delays, and debug logging.
 *
 * @copyright
 * GNU GENERAL PUBLIC LICENSE
 */

#ifndef PLATFORM_HAL_H__
#define PLATFORM_HAL_H__

/*********************************************/
/*                Includes                   */
/*********************************************/

// nRF5 SDK specific includes
#include "app_error.h"
#include "app_timer.h"
#include "app_util_platform.h"
#include "boards.h"
#include "nrf_delay.h"
#include "nrf_drv_spi.h"
#include "nrf_drv_twi.h"
#include "nrf_gpio.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

// Standard Library Includes
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "math.h"

/*********************************************/
/*                Defines                    */
/*********************************************/

// --- Debug Print Configuration ---
// Define this to enable logging from the HAL layer. Comment it out to disable.
#define PLATFORM_LOG_INFO_ENABLED
#define PLATFORM_LOG_ERROR_ENABLED
#define PLATFORM_LOG_WARNING_ENABLED
//#define PLATFORM_LOG_DEBUG_ENABLED

#ifdef PLATFORM_LOG_INFO_ENABLED
#define PLATFORM_LOG_INFO(...)                                                                                         \
    NRF_LOG_INFO(__VA_ARGS__);                                                                                         \
    NRF_LOG_FLUSH();
#else
#define PLATFORM_LOG_INFO(...)
#endif

#ifdef PLATFORM_LOG_ERROR_ENABLED
#define PLATFORM_LOG_ERROR(...)                                                                                        \
    NRF_LOG_ERROR(__VA_ARGS__);                                                                                        \
    NRF_LOG_FLUSH();
#else
#define PLATFORM_LOG_ERROR(...)
#endif

#ifdef PLATFORM_LOG_WARNING_ENABLED
#define PLATFORM_LOG_WARNING(...)                                                                                      \
    NRF_LOG_WARNING(__VA_ARGS__);                                                                                      \
    NRF_LOG_FLUSH();
#else
#define PLATFORM_LOG_WARNING(...)
#endif

#ifdef PLATFORM_LOG_DEBUG_ENABLED
#define PLATFORM_LOG_DEBUG(...)                                                                                        \
    NRF_LOG_DEBUG(__VA_ARGS__);                                                                                        \
    NRF_LOG_FLUSH();
#else
#define PLATFORM_LOG_DEBUG(...)
#endif

#define SPI_INSTANCE 1

#define SPI_SCK_PIN 11
#define SPI_MISO_PIN 12
#define SPI_MOSI_PIN 13
#define ICM20948_SS_PIN 14

#define TWI_INSTANCE 0

#define TWI_SCL_PIN 11
#define TWI_SDA_PIN 13
#define ICM20948_I2C_ADDR 0x69

// --- HAL Configuration ---
#define PLATFORM_HAL_MAX_BUF_SIZE 256

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

/**
 * @brief TODO
 */
typedef enum
{
    COMM_PROTOCOL_SPI, /**< TODO */
    COMM_PROTOCOL_TWI  /**< TODO */
} comm_protocol_t;

/**
 * @brief TODO
 */
typedef enum
{
    PLATFORM_LOG_LEVEL_ERROR = (1 << 0),
    PLATFORM_LOG_LEVEL_WARNING = (1 << 1),
    PLATFORM_LOG_LEVEL_INFO = (1 << 2),
    PLATFORM_LOG_LEVEL_DEBUG = (1 << 3),
} platform_log_level_t;

/*********************************************/
/*                Unions                     */
/*********************************************/

/**
 * @brief Configuration for communication mode and its parameters.
 */
typedef struct
{
    comm_protocol_t protocol; /**< The selected communication protocol (SPI/TWI) */
    comm_mode_t mode;         /**< The selected communication mode */
    int32_t timeout_ms;       /**< Timeout duration in milliseconds (used in blocking mode) */
} comm_cfg_t;

/**
 * @brief Union for SPI or TWI/I2C bus configuration.
 *
 * Only one member should be used depending on the bus type.
 */
typedef union {
    struct
    {
        void *p_instance;            ///< Pointer to SPI peripheral instance
        uint8_t ss_pin;              ///< SPI Slave Select pin
        volatile bool spi_xfer_done; // Volatile flag to wait for SPI transfers to complete.
    } spi;

    struct
    {
        void *p_instance; ///< Pointer to TWI/I2C peripheral instance
        uint8_t twi_addr; ///< 7-bit I2C slave address
        volatile bool twi_xfer_done;
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
typedef platform_hal_err_t (*platform_hal_write_reg_t)(platform_hal_bus_cfg_t *p_bus_cfg, uint8_t reg,
                                                       const uint8_t *p_data, uint32_t len,
                                                       const comm_cfg_t *p_comm_cfg);

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
 * @brief Platform-specific delay in microseconds.
 * @param us Number of microseconds to delay.
 */
void platform_hal_delay_us(uint32_t us);

/**
 * @brief Returns the current system tick count.
 * Requires app_timer to be initialized.
 */
uint32_t platform_hal_get_ticks();

/**
 * @brief Platform-specific initialisation.
 * @param TODO
 */
platform_hal_err_t platform_hal_init(comm_cfg_t *comm_cfg, platform_hal_bus_cfg_t *bus_cfg);

/**
 * @brief SPI event handler for asynchronous transfers.
 * @note This function should be registered as the event handler when initializing the SPI driver.
 */
void platform_hal_spi_event_handler(nrf_drv_spi_evt_t const *p_event, void *p_context);

/**
 * @brief HAL implementation for writing data over SPI.
 */
platform_hal_err_t platform_hal_spi_write(platform_hal_bus_cfg_t *p_bus_cfg, uint8_t reg, const uint8_t *p_data,
                                          uint32_t len, const comm_cfg_t *p_comm_cfg);

/**
 * @brief HAL implementation for reading data over SPI.
 */
platform_hal_err_t platform_hal_spi_read(platform_hal_bus_cfg_t *p_bus_cfg, uint8_t reg, uint8_t *p_data, uint32_t len,
                                         const comm_cfg_t *p_comm_cfg);
/**
 * @brief TWI event handler for asynchronous transfers.
 * @note This function should be registered as the event handler when initializing the TWI driver.
 */
void platform_hal_twi_event_handler(nrf_drv_twi_evt_t const *p_event, void *p_context);

/**
 * @brief HAL implementation for writing data over TWI/I2C.
 */
platform_hal_err_t platform_hal_twi_write(platform_hal_bus_cfg_t *p_bus_cfg, uint8_t reg, const uint8_t *p_data,
                                          uint32_t len, const comm_cfg_t *p_comm_cfg);

/**
 * @brief HAL implementation for reading data over TWI/I2C.
 */
platform_hal_err_t platform_hal_twi_read(platform_hal_bus_cfg_t *p_bus_cfg, uint8_t reg, uint8_t *p_data, uint32_t len,
                                         const comm_cfg_t *p_comm_cfg);

#endif // PLATFORM_HAL_H__
