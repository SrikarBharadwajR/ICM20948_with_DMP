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

#include "platform_hal_nrf52.h"

/*********************************************/
/*               Functions                   */
/*********************************************/

void platform_hal_delay_ms(uint32_t ms)
{
    nrf_delay_ms(ms);
}

void platform_hal_delay_us(uint32_t us)
{
    nrf_delay_us(us);
}
/**
 * @brief Returns the current system tick count.
 * Requires app_timer to be initialized.
 */

uint32_t platform_hal_get_ticks()
{
    return app_timer_cnt_get();
}

platform_hal_err_t platform_hal_init(comm_cfg_t *comm_cfg, platform_hal_bus_cfg_t *bus_cfg)
{
    ret_code_t err_code;

    /* Initialize logging and timers, common to all protocols */
    err_code = NRF_LOG_INIT(NULL);
    if (err_code != NRF_SUCCESS && err_code != NRF_ERROR_INVALID_STATE)
        return PLATFORM_HAL_ERR_COMM;
    NRF_LOG_DEFAULT_BACKENDS_INIT();

    err_code = app_timer_init();
    if (err_code != NRF_SUCCESS && err_code != NRF_ERROR_INVALID_STATE)
        return PLATFORM_HAL_ERR_COMM;

    PLATFORM_LOG_INFO("Platform HAL Initializing...");

    /* Communication Initialise*/
    switch (comm_cfg->protocol)
    {
    case COMM_PROTOCOL_SPI:
        // nRF SPI driver instance
        static const nrf_drv_spi_t m_spi = NRF_DRV_SPI_INSTANCE(SPI_INSTANCE);
        nrf_drv_spi_config_t spi_config;
        spi_config.ss_pin = NRF_DRV_SPI_PIN_NOT_USED;
        spi_config.miso_pin = SPI_MISO_PIN;
        spi_config.mosi_pin = SPI_MOSI_PIN;
        spi_config.sck_pin = SPI_SCK_PIN;
        spi_config.irq_priority = 1;
        spi_config.orc = 0xFF;
        spi_config.frequency = NRF_DRV_SPI_FREQ_125K;
        spi_config.mode = NRF_DRV_SPI_MODE_0;
        spi_config.bit_order = NRF_DRV_SPI_BIT_ORDER_MSB_FIRST;

        // The platform_hal_spi_event_handler is required by the HAL implementation
        err_code = nrf_drv_spi_init(&m_spi, &spi_config, platform_hal_spi_event_handler, (void *)bus_cfg);
        if (err_code != NRF_SUCCESS)
            return PLATFORM_HAL_ERR_COMM;

        nrf_gpio_cfg_output(ICM20948_SS_PIN);
        nrf_gpio_pin_set(ICM20948_SS_PIN);

        // Configure the bus-specific details
        bus_cfg->spi.p_instance = (void *)&m_spi;

        break;

    case COMM_PROTOCOL_TWI: {
        static const nrf_drv_twi_t m_twi = NRF_DRV_TWI_INSTANCE(TWI_INSTANCE);

        nrf_drv_twi_config_t twi_config;
        twi_config.scl = TWI_SCL_PIN;
        twi_config.sda = TWI_SDA_PIN;
        twi_config.frequency = NRF_DRV_TWI_FREQ_100K;
        twi_config.interrupt_priority = 6;
        twi_config.clear_bus_init = false;
        twi_config.hold_bus_uninit = false;

        err_code = nrf_drv_twi_init(&m_twi, &twi_config, platform_hal_twi_event_handler, (void *)bus_cfg);

        if (err_code != NRF_SUCCESS)
            return PLATFORM_HAL_ERR_COMM;

        nrf_drv_twi_enable(&m_twi);
        bus_cfg->twi.p_instance = (void *)&m_twi;
        break;
    }
    default:
        return PLATFORM_HAL_ERR_INVALID_PARAM;
    }
    return PLATFORM_HAL_OK;
}

void platform_hal_spi_event_handler(nrf_drv_spi_evt_t const *p_event, void *p_context)
{
    if (p_event->type == NRF_DRV_SPI_EVENT_DONE)
    {
        platform_hal_bus_cfg_t *bus_cfg = (platform_hal_bus_cfg_t *)p_context;
        bus_cfg->spi.spi_xfer_done = true;
    }
}

platform_hal_err_t platform_hal_spi_write(platform_hal_bus_cfg_t *p_bus_cfg, uint8_t reg, const uint8_t *p_data,
                                          uint32_t len, const comm_cfg_t *p_comm_cfg)
{
    if (p_bus_cfg == NULL || p_comm_cfg == NULL)
        return PLATFORM_HAL_ERR_INVALID_PARAM;

    // The p_instance from the union is cast to the nRF driver type
    nrf_drv_spi_t const *p_spi_instance = (nrf_drv_spi_t const *)p_bus_cfg->spi.p_instance;

    if (len + 1 > PLATFORM_HAL_MAX_BUF_SIZE)
    {
        PLATFORM_LOG_ERROR("SPI write length %lu exceeds max %d", len, PLATFORM_HAL_MAX_BUF_SIZE);
        return PLATFORM_HAL_ERR_INVALID_PARAM;
    }

    uint8_t tx_buf[PLATFORM_HAL_MAX_BUF_SIZE];

    tx_buf[0] = reg & 0x7F; // For a write, the MSB is 0.
    memcpy(&tx_buf[1], p_data, len);

    nrf_gpio_pin_clear(p_bus_cfg->spi.ss_pin);

    PLATFORM_LOG_DEBUG("[HAL SPI Write] Reg: 0x%02X, Len: %lu\r", reg, len);

    p_bus_cfg->spi.spi_xfer_done = false;
    ret_code_t err_code = nrf_drv_spi_transfer(p_spi_instance, tx_buf, len + 1, NULL, 0);

    if (err_code != NRF_SUCCESS)
    {
        PLATFORM_LOG_ERROR("[HAL SPI Write] Error: %lu\r", err_code);
        nrf_gpio_pin_set(p_bus_cfg->spi.ss_pin); // Ensure CS is de-asserted on error
        return PLATFORM_HAL_ERR_COMM;
    }

    if (p_comm_cfg->mode == COMM_MODE_BLOCKING)
    {

        uint32_t start_ticks = platform_hal_get_ticks();

        while (!p_bus_cfg->spi.spi_xfer_done)
        {
            if ((platform_hal_get_ticks() - start_ticks) > APP_TIMER_TICKS(p_comm_cfg->timeout_ms))
            {
                nrf_gpio_pin_set(p_bus_cfg->spi.ss_pin);
                return PLATFORM_HAL_ERR_TIMEOUT;
            }
            __WFE();
        }
        nrf_gpio_pin_set(p_bus_cfg->spi.ss_pin);
    }

    else if (p_comm_cfg->mode == COMM_MODE_NON_BLOCKING)
    {
        nrf_gpio_pin_set(p_bus_cfg->spi.ss_pin);
    }
    return PLATFORM_HAL_OK;
}

platform_hal_err_t platform_hal_spi_read(platform_hal_bus_cfg_t *p_bus_cfg, uint8_t reg, uint8_t *p_data, uint32_t len,
                                         const comm_cfg_t *p_comm_cfg)
{
    if (p_bus_cfg == NULL || p_comm_cfg == NULL)
        return PLATFORM_HAL_ERR_INVALID_PARAM;

    nrf_drv_spi_t const *p_spi_instance = (nrf_drv_spi_t const *)p_bus_cfg->spi.p_instance;

    if (len + 1 > PLATFORM_HAL_MAX_BUF_SIZE)
    {
        PLATFORM_LOG_ERROR("SPI read length %lu exceeds max %d", len, PLATFORM_HAL_MAX_BUF_SIZE);
        return PLATFORM_HAL_ERR_INVALID_PARAM;
    }

    uint8_t tx_buf[PLATFORM_HAL_MAX_BUF_SIZE];
    uint8_t rx_buf[PLATFORM_HAL_MAX_BUF_SIZE];

    tx_buf[0] = reg | 0x80; // For a read, the MSB is 1.
    memset(&tx_buf[1], 0, len);

    p_bus_cfg->spi.spi_xfer_done = false;
    PLATFORM_LOG_DEBUG("[HAL SPI Read] Reg: 0x%02X, Len: %lu\r", reg, len);

    nrf_gpio_pin_clear(p_bus_cfg->spi.ss_pin);
    ret_code_t err_code = nrf_drv_spi_transfer(p_spi_instance, tx_buf, len + 1, rx_buf, len + 1);

    if (err_code != NRF_SUCCESS)
    {
        PLATFORM_LOG_ERROR("[HAL SPI Read] Error: %lu\r", err_code);
        nrf_gpio_pin_set(p_bus_cfg->spi.ss_pin); // Ensure CS is de-asserted on error
        return PLATFORM_HAL_ERR_COMM;
    }

    if (p_comm_cfg->mode == COMM_MODE_BLOCKING)
    {
        uint32_t start_ticks = platform_hal_get_ticks();
        while (!p_bus_cfg->spi.spi_xfer_done)
        {
            if ((platform_hal_get_ticks() - start_ticks) > APP_TIMER_TICKS(p_comm_cfg->timeout_ms))
            {
                // TODO: Abort the transfer if possible. For now, just return timeout.
                nrf_gpio_pin_set(p_bus_cfg->spi.ss_pin);
                return PLATFORM_HAL_ERR_TIMEOUT;
            }
            __WFE();
        }
        nrf_gpio_pin_set(p_bus_cfg->spi.ss_pin);
    }

    else if (p_comm_cfg->mode == COMM_MODE_NON_BLOCKING)
    { // Non-blocking
        // User must check p_bus_cfg->spi.spi_xfer_done later.
        // CS pin will be raised when user calls the function again. This is not ideal.
        // For true non-blocking, a state machine is needed. For now, we return.
        nrf_gpio_pin_set(p_bus_cfg->spi.ss_pin); // Raise CS immediately in simple non-blocking
    }

    memcpy(p_data, &rx_buf[1], len);

    return PLATFORM_HAL_OK;
}

void platform_hal_twi_event_handler(nrf_drv_twi_evt_t const *p_event, void *p_context)
{
    if (p_event->type == NRF_DRV_TWI_EVT_DONE)
    {
        platform_hal_bus_cfg_t *bus_cfg = (platform_hal_bus_cfg_t *)p_context;
        bus_cfg->twi.twi_xfer_done = true;
    }
}

platform_hal_err_t platform_hal_twi_write(platform_hal_bus_cfg_t *p_bus_cfg, uint8_t reg, const uint8_t *p_data,
                                          uint32_t len, const comm_cfg_t *p_comm_cfg)
{
    if (p_bus_cfg == NULL || p_comm_cfg == NULL)
        return PLATFORM_HAL_ERR_INVALID_PARAM;
    if (len + 1 > PLATFORM_HAL_MAX_BUF_SIZE)
        return PLATFORM_HAL_ERR_INVALID_PARAM;

    nrf_drv_twi_t const *p_twi_instance = (nrf_drv_twi_t const *)p_bus_cfg->twi.p_instance;
    uint8_t tx_buf[PLATFORM_HAL_MAX_BUF_SIZE];
    tx_buf[0] = reg; // First byte is the register address
    memcpy(&tx_buf[1], p_data, len);

    p_bus_cfg->twi.twi_xfer_done = false;
    ret_code_t err_code = nrf_drv_twi_tx(p_twi_instance, p_bus_cfg->twi.twi_addr, tx_buf, len + 1, false);

    if (err_code != NRF_SUCCESS)
        return PLATFORM_HAL_ERR_COMM;

    // nRF TWI driver is blocking by default, but we use the flag for consistency
    // and if we switch to non-blocking driver later.
    if (p_comm_cfg->mode == COMM_MODE_BLOCKING)
    {
        uint32_t start_ticks = platform_hal_get_ticks();
        while (!p_bus_cfg->twi.twi_xfer_done)
        {
            if ((platform_hal_get_ticks() - start_ticks) > APP_TIMER_TICKS(p_comm_cfg->timeout_ms))
            {
                return PLATFORM_HAL_ERR_TIMEOUT;
            }
            __WFE();
        }
    }

    return (err_code == NRF_SUCCESS) ? PLATFORM_HAL_OK : PLATFORM_HAL_ERR_COMM;
}

platform_hal_err_t platform_hal_twi_read(platform_hal_bus_cfg_t *p_bus_cfg, uint8_t reg, uint8_t *p_data, uint32_t len,
                                         const comm_cfg_t *p_comm_cfg)
{
    if (p_bus_cfg == NULL || p_comm_cfg == NULL)
        return PLATFORM_HAL_ERR_INVALID_PARAM;

    nrf_drv_twi_t const *p_twi_instance = (nrf_drv_twi_t const *)p_bus_cfg->twi.p_instance;

    // First, write the register address to read from, with no stop condition.
    p_bus_cfg->twi.twi_xfer_done = false;
    ret_code_t err_code = nrf_drv_twi_tx(p_twi_instance, p_bus_cfg->twi.twi_addr, &reg, 1, true);
    if (err_code != NRF_SUCCESS)
        return PLATFORM_HAL_ERR_COMM;

    if (p_comm_cfg->mode == COMM_MODE_BLOCKING)
    {
        while (!p_bus_cfg->twi.twi_xfer_done)
        {
            __WFE();
        } // Wait for TX to finish
    }

    // Now, read the data from the sensor.
    p_bus_cfg->twi.twi_xfer_done = false;
    err_code = nrf_drv_twi_rx(p_twi_instance, p_bus_cfg->twi.twi_addr, p_data, len);
    if (err_code != NRF_SUCCESS)
        return PLATFORM_HAL_ERR_COMM;

    if (p_comm_cfg->mode == COMM_MODE_BLOCKING)
    {
        uint32_t start_ticks = platform_hal_get_ticks();
        while (!p_bus_cfg->twi.twi_xfer_done)
        {
            if ((platform_hal_get_ticks() - start_ticks) > APP_TIMER_TICKS(p_comm_cfg->timeout_ms))
            {
                return PLATFORM_HAL_ERR_TIMEOUT;
            }
            __WFE();
        }
    }

    return (err_code == NRF_SUCCESS) ? PLATFORM_HAL_OK : PLATFORM_HAL_ERR_COMM;
}
