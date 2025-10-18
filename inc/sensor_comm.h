/**
 * @file sensor_comm.h
 * @author Srikar Bharadwaj R
 *
 * @brief Generic Sensor Communication Interface Layer.
 *
 * @details This layer provides a generic, device-agnostic interface for
 * interacting with a sensor over a communication bus. It handles
 * common operations like reading/writing bytes, words, and bits,
 * making the high-level driver code cleaner and more readable.
 * This entire layer is platform-agnostic and can be reused for any
 * sensor driver.
 *
 * @copyright
 * GNU GENERAL PUBLIC LICENSE
 */

#ifndef SENSOR_COMM_H__
#define SENSOR_COMM_H__

/*********************************************/
/*                Includes                   */
/*********************************************/

#include "platform_hal.h"


/*********************************************/
/*                Structures                 */
/*********************************************/

/**
 * @brief Generic sensor communication interface handle.
 * This struct holds the function pointers and platform-specific handle
 * needed to communicate with a device.
 */
typedef struct
{
    platform_hal_bus_cfg_t bus_cfg;     ///< Union holding the platform-specific bus config.
    comm_cfg_t comm_cfg;                ///< Struct holding the communication mode config.
    platform_hal_read_reg_t read_reg;   ///< Pointer to the bus read function.
    platform_hal_write_reg_t write_reg; ///< Pointer to the bus write function.
} sensor_comm_t;

/*********************************************/
/*           Function Prototypes             */
/*********************************************/

platform_hal_err_t sensor_comm_read_byte(sensor_comm_t *p_dev, uint8_t reg_addr, uint8_t *p_data);
platform_hal_err_t sensor_comm_read_word(sensor_comm_t *p_dev, uint8_t reg_addr, uint16_t *p_data);
platform_hal_err_t sensor_comm_read_bytes(sensor_comm_t *p_dev, uint8_t reg_addr, uint8_t *p_data, uint32_t len);

platform_hal_err_t sensor_comm_write_byte(sensor_comm_t *p_dev, uint8_t reg_addr, uint8_t data);
platform_hal_err_t sensor_comm_write_word(sensor_comm_t *p_dev, uint8_t reg_addr, uint16_t data);
platform_hal_err_t sensor_comm_write_bytes(sensor_comm_t *p_dev, uint8_t reg_addr, const uint8_t *p_data, uint32_t len);

platform_hal_err_t sensor_comm_read_bits(sensor_comm_t *p_dev, uint8_t reg_addr, uint8_t bit_start, uint8_t length,
                                        uint8_t *p_data);
platform_hal_err_t sensor_comm_write_bits(sensor_comm_t *p_dev, uint8_t reg_addr, uint8_t bit_start, uint8_t length,
                                         uint8_t data);

#endif // SENSOR_COMM_H__
