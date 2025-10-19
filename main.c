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
    icm20948_err_t err_code = icm20948_init(&icm_dev, &icm_config);

    PLATFORM_LOG_INFO("Initializing ICM20948 driver...");

    if (err_code != ICM20948_OK)
    {
        PLATFORM_LOG_ERROR("Failed to initialize ICM20948 driver. Error code: %d", err_code);
        while (1)
            ; // Halt on error
    }
    PLATFORM_LOG_INFO("ICM20948 driver initialized successfully!");

    PLATFORM_LOG_INFO("Initializing DMP...");
    err_code = icm20948_initialize_dmp(&icm_dev);
    if (err_code != ICM20948_OK)
    {
        PLATFORM_LOG_ERROR("Failed to initialize DMP. Error code: %d", err_code);
        while (1)
            ; // Halt on error
    }
    PLATFORM_LOG_INFO("DMP Initialized.");

    PLATFORM_LOG_INFO("Configuring DMP Sensors...");
    bool success = true;

    // Enable the DMP orientation sensor (Quat9)
    err_code = icm20948_enable_dmp_sensor(&icm_dev, INV_ICM20948_SENSOR_ORIENTATION, true);
    if (err_code != ICM20948_OK)
        success = false;

    // Set ODR for Quat9 to max rate (0 interval)
    err_code = icm20948_set_dmp_odr_rate(&icm_dev, DMP_ODR_Reg_Quat9, 0);
    if (err_code != ICM20948_OK)
        success = false;

    // Enable the FIFO
    err_code = icm20948_enable_fifo(&icm_dev, true);
    if (err_code != ICM20948_OK)
        success = false;

    // Enable the DMP
    err_code = icm20948_enable_dmp(&icm_dev, true);
    if (err_code != ICM20948_OK)
        success = false;

    // Reset DMP
    err_code = icm20948_reset_dmp(&icm_dev);
    if (err_code != ICM20948_OK)
        success = false;

    // Reset FIFO
    err_code = icm20948_reset_fifo(&icm_dev);
    if (err_code != ICM20948_OK)
        success = false;

    // Check success
    if (success)
    {
        PLATFORM_LOG_INFO("DMP enabled and configured successfully!");
    }
    else
    {
        PLATFORM_LOG_ERROR("Enable DMP failed!");
        while (1)
            ; // Halt on error
    }

    icm_20948_DMP_data_t dmp_data;
    ICM_20948_Status_e read_status;

    while (1)
    {
        // Read any DMP data waiting in the FIFO
        err_code = icm20948_read_dmp_data_from_fifo(&icm_dev, &dmp_data, &read_status);

        if (err_code == ICM20948_OK) // Was valid data available? (Corresponds to Stat_Ok or Stat_FIFOMoreDataAvail)
        {
            if ((dmp_data.header & DMP_header_bitmap_Quat9) > 0) // Check if Quat9 data is present
            {
                // Scale to +/- 1; Q0 is derived
                // Data is scaled by 2^30
                double q1 = ((double)dmp_data.Quat9.Data.Q1) / 1073741824.0;
                double q2 = ((double)dmp_data.Quat9.Data.Q2) / 1073741824.0;
                double q3 = ((double)dmp_data.Quat9.Data.Q3) / 1073741824.0;
                // Ensure the argument to sqrt is non-negative (can happen due to floating point inaccuracies)
                double q1q1_q2q2_q3q3 = (q1 * q1) + (q2 * q2) + (q3 * q3);
                double q0 = (q1q1_q2q2_q3q3 < 1.0) ? sqrt(1.0 - q1q1_q2q2_q3q3) : 0.0;

                // Output in ZaneL's Node.js Quaternion animation tool format
                // Using NRF_LOG_RAW_INFO to prevent extra prefixes/timestamps if desired
                // NRF_LOG_RAW_INFO("{\"quat_w\":%.3f, \"quat_x\":%.3f, \"quat_y\":%.3f, \"quat_z\":%.3f}\n", q0, q1,
                // q2,
                //                 q3);
                NRF_LOG_RAW_INFO("{\"quat_w\":%d, \"quat_x\":%d, \"quat_y\":%d, \"quat_z\":%d}\n", q0 * 100, q1 * 100,
                                 q2 * 100, q3 * 100);
                NRF_LOG_FLUSH(); // Ensure the log is sent out
            }
        }
        else if (err_code == ICM20948_ERR_NO_DATA)
        {
            // No data available, normal operation
        }
        else
        {
            // Handle other errors if necessary
            PLATFORM_LOG_WARNING("Error reading DMP data: %d, Status: %d", err_code, read_status);
        }

        // Delay slightly only if there wasn't more data waiting
        if (read_status != ICM_20948_Stat_FIFOMoreDataAvail)
        {
            platform_hal_delay_ms(10);
        }
    }
}