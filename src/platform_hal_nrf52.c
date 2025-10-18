/**
 * @file platform_hal_nrf52.c
 * @author Srikar Bharadwaj R
 *
 * @brief nRF5 SDK specific implementation of the Platform HAL.
 *
 * @details This file implements the functions defined in platform_hal.h
 * using the nRF5 SDK drivers (nrf_drv_spi, nrf_drv_twi, etc.).
 */

/*********************************************/
/*                Includes                   */
/*********************************************/

#include "platform_hal.h"
// nRF5 SDK specific includes
#include "app_error.h"
#include "app_util_platform.h"
#include "boards.h"
#include "nrf_delay.h"
#include "nrf_drv_spi.h"
#include "nrf_drv_twi.h"
#include "nrf_gpio.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

/*********************************************/
/*            Global Variables               */
/*********************************************/

static volatile bool spi_xfer_done; // Volatile flag to wait for SPI transfers to complete.

/*********************************************/
/*               Functions                   */
/*********************************************/

/**
 * @brief SPI event handler for asynchronous transfers.
 * @note This function should be registered as the event handler when initializing the SPI driver.
 */
void platform_hal_spi_event_handler(nrf_drv_spi_evt_t const *p_event, void *p_context)
{
    if (p_event->type == NRF_DRV_SPI_EVENT_DONE)
    {
        spi_xfer_done = true;
    }
}

void platform_hal_delay_ms(uint32_t ms)
{
    nrf_delay_ms(ms);
}

platform_hal_err_t platform_hal_spi_write(platform_hal_bus_cfg_t *p_bus_cfg, uint8_t reg, const uint8_t *p_data,
                                          uint32_t len, const comm_cfg_t *p_comm_cfg)
{
    if (p_bus_cfg == NULL || p_comm_cfg == NULL)
        return PLATFORM_HAL_ERR_INVALID_PARAM;

    // The p_instance from the union is cast to the nRF driver type
    nrf_drv_spi_t const *p_spi_instance = (nrf_drv_spi_t const *)p_bus_cfg->spi.p_instance;

    uint8_t tx_buf[len + 1];
    tx_buf[0] = reg & 0x7F; // For a write, the MSB is 0.
    memcpy(&tx_buf[1], p_data, len);

    nrf_gpio_pin_clear(p_bus_cfg->spi.ss_pin);
    nrf_delay_us(10);

    PLATFORM_HAL_LOG("[HAL SPI Write] Reg: 0x%02X, Len: %lu\r\n", reg, len);

    spi_xfer_done = false;
    ret_code_t err_code = nrf_drv_spi_transfer(p_spi_instance, tx_buf, len + 1, NULL, 0);

    if (err_code != NRF_SUCCESS)
    {
        PLATFORM_HAL_LOG("[HAL SPI Write] Error: %lu\r\n", err_code);
        nrf_gpio_pin_set(p_bus_cfg->spi.ss_pin); // Ensure CS is de-asserted on error
        return PLATFORM_HAL_ERR_COMM;
    }

    if (p_comm_cfg->mode == COMM_MODE_BLOCKING)
    {
        // For nRF, non-blocking is event-driven. This while loop makes it blocking.
        // A timeout could be implemented here with a counter.
        while (!spi_xfer_done)
        {
            __WFE(); // Wait for event, saves power
        }
    }
    // In non-blocking mode, we would return here and expect the event handler to manage the state.

    nrf_delay_us(10);
    nrf_gpio_pin_set(p_bus_cfg->spi.ss_pin);

    return PLATFORM_HAL_OK;
}

platform_hal_err_t platform_hal_spi_read(platform_hal_bus_cfg_t *p_bus_cfg, uint8_t reg, uint8_t *p_data, uint32_t len,
                                         const comm_cfg_t *p_comm_cfg)
{
    if (p_bus_cfg == NULL || p_comm_cfg == NULL)
        return PLATFORM_HAL_ERR_INVALID_PARAM;

    nrf_drv_spi_t const *p_spi_instance = (nrf_drv_spi_t const *)p_bus_cfg->spi.p_instance;

    uint8_t tx_buf[len + 1];
    uint8_t rx_buf[len + 1];

    tx_buf[0] = reg | 0x80; // For a read, the MSB is 1.
    memset(&tx_buf[1], 0, len);

    nrf_gpio_pin_clear(p_bus_cfg->spi.ss_pin);
    nrf_delay_us(10);

    PLATFORM_HAL_LOG("[HAL SPI Read] Reg: 0x%02X, Len: %lu\r\n", reg, len);

    spi_xfer_done = false;
    ret_code_t err_code = nrf_drv_spi_transfer(p_spi_instance, tx_buf, len + 1, rx_buf, len + 1);

    if (err_code != NRF_SUCCESS)
    {
        PLATFORM_HAL_LOG("[HAL SPI Read] Error: %lu\r\n", err_code);
        nrf_gpio_pin_set(p_bus_cfg->spi.ss_pin); // Ensure CS is de-asserted on error
        return PLATFORM_HAL_ERR_COMM;
    }

    if (p_comm_cfg->mode == COMM_MODE_BLOCKING)
    {
        while (!spi_xfer_done)
        {
            __WFE();
        }
    }

    memcpy(p_data, &rx_buf[1], len);

    nrf_delay_us(10);
    nrf_gpio_pin_set(p_bus_cfg->spi.ss_pin);

    return PLATFORM_HAL_OK;
}

platform_hal_err_t platform_hal_twi_write(platform_hal_bus_cfg_t *p_bus_cfg, uint8_t reg, const uint8_t *p_data,
                                          uint32_t len, const comm_cfg_t *p_comm_cfg)
{
    PLATFORM_HAL_LOG("[HAL TWI Write] Not supported.\r\n");
    return PLATFORM_HAL_ERR_NOT_SUPPORTED;
}

platform_hal_err_t platform_hal_twi_read(platform_hal_bus_cfg_t *p_bus_cfg, uint8_t reg, uint8_t *p_data, uint32_t len,
                                         const comm_cfg_t *p_comm_cfg)
{
    PLATFORM_HAL_LOG("[HAL TWI Read] Not supported.\r\n");
    return PLATFORM_HAL_ERR_NOT_SUPPORTED;
}
