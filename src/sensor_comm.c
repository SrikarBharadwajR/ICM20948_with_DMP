/**
 * @file sensor_comm.c
 * @author Srikar Bharadwaj R
 *
 * @brief Implementation of the generic Sensor Communication Layer.
 */

/*********************************************/
/*                Includes                   */
/*********************************************/

#include "sensor_comm.h"

/*********************************************/
/*               Functions                   */
/*********************************************/

platform_hal_err_t sensor_comm_read_byte(sensor_comm_t *p_dev, uint8_t reg_addr, uint8_t *p_data)
{
    if (p_dev == NULL || p_dev->read_reg == NULL)
        return PLATFORM_HAL_ERR_INVALID_PARAM;
    return p_dev->read_reg(&p_dev->bus_cfg, reg_addr, p_data, 1, &p_dev->comm_cfg);
}

platform_hal_err_t sensor_comm_read_word(sensor_comm_t *p_dev, uint8_t reg_addr, uint16_t *p_data)
{
    if (p_dev == NULL || p_dev->read_reg == NULL)
        return PLATFORM_HAL_ERR_INVALID_PARAM;
    uint8_t buffer[2];
    platform_hal_err_t err_code = p_dev->read_reg(&p_dev->bus_cfg, reg_addr, buffer, 2, &p_dev->comm_cfg);
    if (err_code == PLATFORM_HAL_OK)
    {
        *p_data = ((uint16_t)buffer[0] << 8) | buffer[1];
    }
    return err_code;
}

platform_hal_err_t sensor_comm_read_bytes(sensor_comm_t *p_dev, uint8_t reg_addr, uint8_t *p_data, uint32_t len)
{
    if (p_dev == NULL || p_dev->read_reg == NULL)
        return PLATFORM_HAL_ERR_INVALID_PARAM;
    return p_dev->read_reg(&p_dev->bus_cfg, reg_addr, p_data, len, &p_dev->comm_cfg);
}

platform_hal_err_t sensor_comm_write_byte(sensor_comm_t *p_dev, uint8_t reg_addr, uint8_t data)
{
    if (p_dev == NULL || p_dev->write_reg == NULL)
        return PLATFORM_HAL_ERR_INVALID_PARAM;
    return p_dev->write_reg(&p_dev->bus_cfg, reg_addr, &data, 1, &p_dev->comm_cfg);
}

platform_hal_err_t sensor_comm_write_word(sensor_comm_t *p_dev, uint8_t reg_addr, uint16_t data)
{
    if (p_dev == NULL || p_dev->write_reg == NULL)
        return PLATFORM_HAL_ERR_INVALID_PARAM;
    uint8_t buffer[2] = {(uint8_t)(data >> 8), (uint8_t)(data & 0xFF)};
    return p_dev->write_reg(&p_dev->bus_cfg, reg_addr, buffer, 2, &p_dev->comm_cfg);
}

platform_hal_err_t sensor_comm_write_bytes(sensor_comm_t *p_dev, uint8_t reg_addr, const uint8_t *p_data, uint32_t len)
{
    if (p_dev == NULL || p_dev->write_reg == NULL)
        return PLATFORM_HAL_ERR_INVALID_PARAM;
    return p_dev->write_reg(&p_dev->bus_cfg, reg_addr, p_data, len, &p_dev->comm_cfg);
}

platform_hal_err_t sensor_comm_read_bits(sensor_comm_t *p_dev, uint8_t reg_addr, uint8_t bit_start, uint8_t length,
                                        uint8_t *p_data)
{
    uint8_t byte;
    platform_hal_err_t err_code = sensor_comm_read_byte(p_dev, reg_addr, &byte);
    if (err_code == PLATFORM_HAL_OK)
    {
        uint8_t mask = ((1 << length) - 1) << (bit_start - length + 1);
        byte &= mask;
        byte >>= (bit_start - length + 1);
        *p_data = byte;
    }
    return err_code;
}

platform_hal_err_t sensor_comm_write_bits(sensor_comm_t *p_dev, uint8_t reg_addr, uint8_t bit_start, uint8_t length,
                                         uint8_t data)
{
    uint8_t byte;
    platform_hal_err_t err_code = sensor_comm_read_byte(p_dev, reg_addr, &byte);
    if (err_code != PLATFORM_HAL_OK)
    {
        return err_code;
    }

    uint8_t mask = ((1 << length) - 1) << (bit_start - length + 1);
    data <<= (bit_start - length + 1); // shift data into correct position
    data &= mask;                      // zero all non-important bits in data
    byte &= ~mask;                     // zero all important bits in existing byte
    byte |= data;                      // combine data with existing byte

    return sensor_comm_write_byte(p_dev, reg_addr, byte);
}
