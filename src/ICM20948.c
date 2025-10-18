/**
 * @file ICM20948.c
 * @author Srikar Bharadwaj R
 *
 * @brief Implementation of the high-level ICM20948 driver.
 */

/*********************************************/
/*                Includes                   */
/*********************************************/

#include "ICM20948.h"

/*********************************************/
/*               Functions                   */
/*********************************************/

icm20948_err_t icm20948_init(icm20948_dev_t *p_dev, const icm20948_cfg_t *p_cfg, bool is_spi)
{
    if (p_dev == NULL || p_cfg == NULL)
    {
        return ICM20948_ERR_INVALID_PARAM;
    }

    // Copy the user's configuration into the device's communication handle
    p_dev->comm.bus_cfg = p_cfg->bus_cfg;
    p_dev->comm.comm_cfg = p_cfg->comm_cfg;

    // Assign the correct HAL read/write functions based on the bus type
    if (is_spi)
    {
        p_dev->comm.read_reg = platform_hal_spi_read;
        p_dev->comm.write_reg = platform_hal_spi_write;
    }
    else // TWI
    {
        p_dev->comm.read_reg = platform_hal_twi_read;
        p_dev->comm.write_reg = platform_hal_twi_write;
    }

    // A small delay after configuration might be needed for the sensor to stabilize
    platform_hal_delay_ms(10);

    // Verify the device is present and responding
    return icm20948_who_am_i_check(p_dev);
}

icm20948_err_t icm20948_who_am_i_check(icm20948_dev_t *p_dev)
{
    uint8_t who_am_i_value = 0;

    // Use the generic communication function to read the register
    platform_hal_err_t err_code = sensor_comm_read_byte(&p_dev->comm, ICM20948_WHO_AM_I_REG, &who_am_i_value);

    if (err_code != PLATFORM_HAL_OK)
    {
        // The HAL log will have already printed the specific error
        return ICM20948_ERR_COMM;
    }

    if (who_am_i_value != ICM20948_EXPECTED_WHO_AM_I)
    {
        PLATFORM_HAL_LOG("[ICM20948] WHO_AM_I mismatch. Expected 0x%02X, Got 0x%02X\r\n", 
                         ICM20948_EXPECTED_WHO_AM_I, who_am_i_value);
        return ICM20948_ERR_DEVICE_NOT_FOUND;
    }

    PLATFORM_HAL_LOG("[ICM20948] WHO_AM_I check successful!\r\n");
    return ICM20948_OK;
}
