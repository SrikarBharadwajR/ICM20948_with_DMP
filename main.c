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

#include "ICM20948.h"

#define SPI_INSTANCE 1
#define SPI_SCK_PIN 11
#define SPI_MISO_PIN 12
#define SPI_MOSI_PIN 13
#define ICM20948_SS_PIN 14

// nRF SPI driver instance
static const nrf_drv_spi_t m_spi = NRF_DRV_SPI_INSTANCE(SPI_INSTANCE);

// ICM20948 device handle
static icm20948_dev_t m_icm20948_dev;

/**
 * @brief Function for initializing the SPI driver.
 */
static void spi_init(void)
{
    nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG;
    spi_config.ss_pin = NRF_DRV_SPI_PIN_NOT_USED;
    spi_config.miso_pin = SPI_MISO_PIN;
    spi_config.mosi_pin = SPI_MOSI_PIN;
    spi_config.sck_pin = SPI_SCK_PIN;
    spi_config.frequency = NRF_DRV_SPI_FREQ_125K;

    // The platform_hal_spi_event_handler is required by the HAL implementation
    ret_code_t err_code = nrf_drv_spi_init(&m_spi, &spi_config, platform_hal_spi_event_handler, NULL);
    APP_ERROR_CHECK(err_code);

    // Configure the Slave Select pin as a standard GPIO output
    nrf_gpio_cfg_output(ICM20948_SS_PIN);
    nrf_gpio_pin_set(ICM20948_SS_PIN); // Set SS high (inactive)
}

/**
 * @brief Function for application main entry.
 */
int main(void)
{
    // Initialize logging
    ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);
    NRF_LOG_DEFAULT_BACKENDS_INIT();

    NRF_LOG_INFO("ICM20948 Driver Initialization Example");
    NRF_LOG_FLUSH();

    // Initialize the SPI peripheral
    spi_init();

    // --- Configure the ICM20948 Driver ---

    // 1. Create the main configuration structure
    icm20948_cfg_t icm_config;

    // 2. Configure the communication mode (blocking with a 100ms timeout)
    icm_config.comm_cfg.mode = COMM_MODE_BLOCKING;
    icm_config.comm_cfg.timeout_ms = 100;

    // 3. Configure the bus-specific details (SPI in this case)
    icm_config.bus_cfg.spi.p_instance = (void *)&m_spi; // Pass a pointer to the initialized nRF SPI instance
    icm_config.bus_cfg.spi.ss_pin = ICM20948_SS_PIN;

    while (1)
    {
        // --- Initialize the Driver ---

        NRF_LOG_INFO("Initializing ICM20948 driver...");
        NRF_LOG_FLUSH();

        // Call the init function, passing:
        // - The device handle to be populated
        // - The configuration structure we just created
        // - 'true' because we are using SPI
        icm20948_err_t icm_err = icm20948_init(&m_icm20948_dev, &icm_config, true);

        if (icm_err == ICM20948_OK)
        {
            NRF_LOG_INFO("ICM20948 driver initialized successfully!");
        }
        else
        {
            // The detailed error (WHO_AM_I mismatch, etc.) will have been printed by the HAL/driver log
            NRF_LOG_ERROR("Failed to initialize ICM20948 driver. Error code: %d", icm_err);
        }
        NRF_LOG_FLUSH();
    }
}
