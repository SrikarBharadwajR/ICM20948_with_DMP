#include "ICM20948.h"

/*Main instance of the ICM20948 device handle*/
static icm20948_dev_t icm_dev;

/* Main ICM20948 device configurations handle */
static icm20948_cfg_t icm_config = {
    .comm_cfg = {.protocol = COMM_PROTOCOL_SPI, .mode = COMM_MODE_BLOCKING, .timeout_ms = 50}};

// static icm20948_cfg_t icm_config = {
//     .comm_cfg = {.protocol = COMM_PROTOCOL_TWI, .mode = COMM_MODE_BLOCKING, .timeout_ms = 50}};

/**
 * @brief Function for application main entry.
 */
int main(void)
{
    icm20948_err_t icm_err = icm20948_init(&icm_dev, &icm_config);

    PLATFORM_LOG_INFO("ICM20948 Driver Initialization Example");

    bool success = true; // Use success to show if the DMP configuration was successful

    // Initialize the DMP. initializeDMP is a weak function. You can overwrite it if you want to e.g. to change the
    // sample rate
    // success &= (initializeDMP() == ICM_20948_Stat_Ok);

    if (success)
    {
        PLATFORM_LOG_INFO("DMP Initialization Done");
    }
    else
    {
        PLATFORM_LOG_INFO("DMP Initialization Failed");
    }
    while (1)
    {
        PLATFORM_LOG_INFO("Initializing ICM20948 driver...");

        if (icm20948_who_am_i_check(&icm_dev) == ICM20948_OK)
        {
            PLATFORM_LOG_DEBUG("ICM20948 driver initialized successfully!");
        }
        else
        {
            PLATFORM_LOG_ERROR("Failed to initialize ICM20948 driver. Error code: %d", icm_err);
        }
        platform_hal_delay_ms(10);
    }
}
