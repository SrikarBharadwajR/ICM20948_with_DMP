/**
 * @file ICM20948.c
 * @author Srikar Bharadwaj R (ported from SparkFun C++ library)
 *
 * @brief Implementation of the high-level ICM20948 driver.
 */

/*********************************************/
/*                Includes                   */
/*********************************************/

#include "ICM20948.h"
#include "util/AK09916_REGISTERS.h" // Magnetometer registers
#include "util/ICM_20948_C.h"       // C backbone from original library

/*********************************************/
/*           Function Prototypes             */
/*********************************************/

static icm20948_err_t icm20948_write_reg(icm20948_dev_t *p_dev, uint8_t reg, const uint8_t *p_data, uint32_t len);
static icm20948_err_t icm20948_read_reg(icm20948_dev_t *p_dev, uint8_t reg, uint8_t *p_data, uint32_t len);
static icm20948_err_t icm20948_mag_write_reg(icm20948_dev_t *p_dev, uint8_t reg, uint8_t data);
static icm20948_err_t icm20948_mag_read_reg(icm20948_dev_t *p_dev, uint8_t reg, uint8_t *p_data);

// Wrapper functions to match the required serif signature
static ICM_20948_Status_e serif_write_wrapper(uint8_t reg, uint8_t *pdata, uint32_t len, void *user)
{
    icm20948_dev_t *p_dev = (icm20948_dev_t *)user;
    if (icm20948_write_reg(p_dev, reg, pdata, len) == ICM20948_OK)
    {
        return ICM_20948_Stat_Ok;
    }
    return ICM_20948_Stat_Err;
}

static ICM_20948_Status_e serif_read_wrapper(uint8_t reg, uint8_t *pdata, uint32_t len, void *user)
{
    icm20948_dev_t *p_dev = (icm20948_dev_t *)user;
    if (icm20948_read_reg(p_dev, reg, pdata, len) == ICM20948_OK)
    {
        return ICM_20948_Stat_Ok;
    }
    return ICM_20948_Stat_Err;
}

/*********************************************/
/*               Functions                   */
/*********************************************/

icm20948_err_t icm20948_init(icm20948_dev_t *p_dev, icm20948_cfg_t *p_cfg)
{
    if (p_dev == NULL || p_cfg == NULL)
    {
        return ICM20948_ERR_INVALID_PARAM;
    }

    // 1. Copy the user's configuration into the device's internal communication handle.
    p_dev->comm.bus_cfg = p_cfg->bus_cfg;
    p_dev->comm.comm_cfg = p_cfg->comm_cfg;

    // 2. Assign board-specific hardware details and HAL function pointers.
    if (p_cfg->comm_cfg.protocol == COMM_PROTOCOL_SPI)
    {
        p_dev->comm.bus_cfg.spi.ss_pin = ICM20948_SS_PIN;
        p_dev->comm.read_reg = platform_hal_spi_read;
        p_dev->comm.write_reg = platform_hal_spi_write;
    }
    else if (p_cfg->comm_cfg.protocol == COMM_PROTOCOL_TWI)
    {
        p_dev->comm.bus_cfg.twi.twi_addr = ICM20948_I2C_ADDR;
        p_dev->comm.read_reg = platform_hal_twi_read;
        p_dev->comm.write_reg = platform_hal_twi_write;
    }
    else
    {
        return ICM20948_ERR_INVALID_PARAM;
    }

    // 3. Initialise the platform hardware (SPI/TWI bus, timers, etc.).
    if (platform_hal_init(&p_dev->comm.comm_cfg, &p_dev->comm.bus_cfg) != PLATFORM_HAL_OK)
    {
        return ICM20948_ERR_PLATFORM_FAIL;
    }
    // Initialize the C backbone driver structure
    ICM_20948_init_struct(&p_dev->c_dev);

    p_dev->c_dev._dmp_firmware_available = true;

    // Link our platform-specific read/write functions to the C backbone via the serif struct
    p_dev->c_dev._serif = &p_dev->serif;      // Point the C-backbone's serif pointer to our instance
    p_dev->serif.write = serif_write_wrapper; // Assign our write wrapper
    p_dev->serif.read = serif_read_wrapper;   // Assign our read wrapper
    p_dev->serif.user = p_dev;                // Pass a pointer to our main device struct as the user pointer

    // 4. Wait for the sensor to power up.
    platform_hal_delay_ms(100);

    // 5. Perform the default startup sequence.
    if (icm20948_startup_default(p_dev) != ICM20948_OK)
    {
        PLATFORM_LOG_ERROR("[ICM20948] Default startup failed.\r");
        return ICM20948_ERR_COMM;
    }

    return ICM20948_OK;
}

icm20948_err_t icm20948_who_am_i_check(icm20948_dev_t *p_dev)
{
    ICM_20948_Status_e status = ICM_20948_check_id(&p_dev->c_dev);
    if (status != ICM_20948_Stat_Ok)
    {
        PLATFORM_LOG_ERROR("[ICM20948] WHO_AM_I mismatch.\r");
        return ICM20948_ERR_DEVICE_NOT_FOUND;
    }
    PLATFORM_LOG_DEBUG("[ICM20948] WHO_AM_I check successful!\r");
    return ICM20948_OK;
}

icm20948_err_t icm20948_get_agmt(icm20948_dev_t *p_dev, ICM_20948_AGMT_t *p_agmt)
{
    if (ICM_20948_get_agmt(&p_dev->c_dev, p_agmt) != ICM_20948_Stat_Ok)
    {
        return ICM20948_ERR_COMM;
    }
    return ICM20948_OK;
}

icm20948_err_t icm20948_sw_reset(icm20948_dev_t *p_dev)
{
    if (ICM_20948_sw_reset(&p_dev->c_dev) != ICM_20948_Stat_Ok)
    {
        return ICM20948_ERR_COMM;
    }
    return ICM20948_OK;
}

icm20948_err_t icm20948_sleep(icm20948_dev_t *p_dev, bool on)
{
    if (ICM_20948_sleep(&p_dev->c_dev, on) != ICM_20948_Stat_Ok)
    {
        return ICM20948_ERR_COMM;
    }
    return ICM20948_OK;
}

icm20948_err_t icm20948_low_power(icm20948_dev_t *p_dev, bool on)
{
    if (ICM_20948_low_power(&p_dev->c_dev, on) != ICM_20948_Stat_Ok)
    {
        return ICM20948_ERR_COMM;
    }
    return ICM20948_OK;
}

icm20948_err_t icm20948_set_clock_source(icm20948_dev_t *p_dev, ICM_20948_PWR_MGMT_1_CLKSEL_e source)
{
    if (ICM_20948_set_clock_source(&p_dev->c_dev, source) != ICM_20948_Stat_Ok)
    {
        return ICM20948_ERR_COMM;
    }
    return ICM20948_OK;
}

icm20948_err_t icm20948_startup_default(icm20948_dev_t *p_dev)
{
    icm20948_err_t err_code = ICM20948_OK;

    err_code = icm20948_who_am_i_check(p_dev);
    if (err_code != ICM20948_OK)
        return err_code;

    err_code = icm20948_sw_reset(p_dev);
    if (err_code != ICM20948_OK)
        return err_code;
    platform_hal_delay_ms(50);

    err_code = icm20948_sleep(p_dev, false);
    if (err_code != ICM20948_OK)
        return err_code;

    err_code = icm20948_low_power(p_dev, false);
    if (err_code != ICM20948_OK)
        return err_code;

    err_code = icm20948_startup_magnetometer(p_dev, false);
    if (err_code != ICM20948_OK)
        return err_code;

    if (ICM_20948_set_sample_mode(&p_dev->c_dev,
                                  (ICM_20948_InternalSensorID_bm)(ICM_20948_Internal_Acc | ICM_20948_Internal_Gyr),
                                  ICM_20948_Sample_Mode_Continuous) != ICM_20948_Stat_Ok)
    {
        return ICM20948_ERR_COMM;
    }

    ICM_20948_fss_t fss;
    fss.a = gpm2;   // 2g
    fss.g = dps250; // 250 dps
    if (ICM_20948_set_full_scale(&p_dev->c_dev,
                                 (ICM_20948_InternalSensorID_bm)(ICM_20948_Internal_Acc | ICM_20948_Internal_Gyr),
                                 fss) != ICM_20948_Stat_Ok)
    {
        return ICM20948_ERR_COMM;
    }

    ICM_20948_dlpcfg_t dlpcfg;
    dlpcfg.a = acc_d473bw_n499bw;
    dlpcfg.g = gyr_d361bw4_n376bw5;
    if (ICM_20948_set_dlpf_cfg(&p_dev->c_dev,
                               (ICM_20948_InternalSensorID_bm)(ICM_20948_Internal_Acc | ICM_20948_Internal_Gyr),
                               dlpcfg) != ICM_20948_Stat_Ok)
    {
        return ICM20948_ERR_COMM;
    }

    if (ICM_20948_enable_dlpf(&p_dev->c_dev, ICM_20948_Internal_Acc, false) != ICM_20948_Stat_Ok)
        return ICM20948_ERR_COMM;
    if (ICM_20948_enable_dlpf(&p_dev->c_dev, ICM_20948_Internal_Gyr, false) != ICM_20948_Stat_Ok)
        return ICM20948_ERR_COMM;

    return ICM20948_OK;
}

icm20948_err_t icm20948_startup_magnetometer(icm20948_dev_t *p_dev, bool minimal)
{
    ICM_20948_i2c_master_passthrough(&p_dev->c_dev, false);
    ICM_20948_i2c_master_enable(&p_dev->c_dev, true);

    icm20948_reset_mag(p_dev);

    uint8_t tries = 0;
    while (tries < 10)
    {
        tries++;
        if (icm20948_mag_who_i_am(p_dev) == ICM20948_OK)
            break;
        ICM_20948_i2c_master_reset(&p_dev->c_dev);
        platform_hal_delay_ms(10);
    }

    if (tries == 10)
    {
        return ICM20948_ERR_DEVICE_NOT_FOUND;
    }

    if (minimal)
    {
        return ICM20948_OK;
    }

    AK09916_CNTL2_Reg_t reg;
    reg.MODE = AK09916_mode_cont_100hz;
    reg.reserved_0 = 0;
    if (icm20948_mag_write_reg(p_dev, AK09916_REG_CNTL2, *(uint8_t *)&reg) != ICM20948_OK)
    {
        return ICM20948_ERR_COMM;
    }

    if (ICM_20948_i2c_controller_configure_peripheral(&p_dev->c_dev, 0, MAG_AK09916_I2C_ADDR, AK09916_REG_ST1, 9, true,
                                                      true, false, false, false, 0) != ICM_20948_Stat_Ok)
    {
        return ICM20948_ERR_COMM;
    }

    return ICM20948_OK;
}

icm20948_err_t icm20948_mag_who_i_am(icm20948_dev_t *p_dev)
{
    uint8_t wia1 = 0, wia2 = 0;
    if (icm20948_mag_read_reg(p_dev, AK09916_REG_WIA1, &wia1) != ICM20948_OK)
    {
        return ICM20948_ERR_COMM;
    }
    if (icm20948_mag_read_reg(p_dev, AK09916_REG_WIA2, &wia2) != ICM20948_OK)
    {
        return ICM20948_ERR_COMM;
    }

    if ((wia1 == (MAG_AK09916_WHO_AM_I >> 8)) && (wia2 == (MAG_AK09916_WHO_AM_I & 0xFF)))
    {
        return ICM20948_OK;
    }

    return ICM20948_ERR_DEVICE_NOT_FOUND;
}

icm20948_err_t icm20948_reset_mag(icm20948_dev_t *p_dev)
{
    uint8_t SRST = 1;
    return icm20948_mag_write_reg(p_dev, AK09916_REG_CNTL3, SRST);
}

icm20948_err_t icm20948_initialize_dmp(icm20948_dev_t *p_dev)
{
    // First, check if DMP is available
    if (p_dev->c_dev._dmp_firmware_available != true)
    {
        PLATFORM_LOG_ERROR("[ICM20948] DMP not available. ICM_20948_USE_DMP is not defined in ICM_20948_C.h");
        return ICM20948_ERR_NOT_SUPPORTED;
    }

    ICM_20948_Status_e result = ICM_20948_Stat_Ok;

    // The ICM-20948 is awake and ready but hasn't been configured for the DMP.
    // This sequence is ported from InvenSense's application note.

    // Configure I2C Master interface for magnetometer
    // SLV0 for data reading
    result = ICM_20948_i2c_controller_configure_peripheral(&p_dev->c_dev, 0, MAG_AK09916_I2C_ADDR, AK09916_REG_RSV2, 10,
                                                           true, true, false, true, true, 0);
    if (result != ICM_20948_Stat_Ok)
        return ICM20948_ERR_COMM;

    // SLV1 for single measurement triggering
    result = ICM_20948_i2c_controller_configure_peripheral(&p_dev->c_dev, 1, MAG_AK09916_I2C_ADDR, AK09916_REG_CNTL2, 1,
                                                           false, true, false, false, false, AK09916_mode_single);
    if (result != ICM_20948_Stat_Ok)
        return ICM20948_ERR_COMM;

    // Set I2C Master ODR configuration
    result = ICM_20948_set_bank(&p_dev->c_dev, 3);
    if (result != ICM_20948_Stat_Ok)
        return ICM20948_ERR_COMM;
    uint8_t mst_odr_config = 0x04; // 68.75Hz
    result = ICM_20948_execute_w(&p_dev->c_dev, AGB3_REG_I2C_MST_ODR_CONFIG, &mst_odr_config, 1);
    if (result != ICM_20948_Stat_Ok)
        return ICM20948_ERR_COMM;

    // Set clock source to auto
    result = ICM_20948_set_clock_source(&p_dev->c_dev, ICM_20948_Clock_Auto);
    if (result != ICM_20948_Stat_Ok)
        return ICM20948_ERR_COMM;

    // Enable accel and gyro
    result = ICM_20948_set_bank(&p_dev->c_dev, 0);
    if (result != ICM_20948_Stat_Ok)
        return ICM20948_ERR_COMM;
    uint8_t pwr_mgmt_2 = 0x40; // Set reserved bit 6
    result = ICM_20948_execute_w(&p_dev->c_dev, AGB0_REG_PWR_MGMT_2, &pwr_mgmt_2, 1);
    if (result != ICM_20948_Stat_Ok)
        return ICM20948_ERR_COMM;

    // Place I2C Master in low power mode
    result = ICM_20948_set_sample_mode(&p_dev->c_dev, ICM_20948_Internal_Mst, ICM_20948_Sample_Mode_Cycled);
    if (result != ICM_20948_Stat_Ok)
        return ICM20948_ERR_COMM;

    // Disable FIFO, then disable DMP
    result = ICM_20948_enable_FIFO(&p_dev->c_dev, false);
    if (result != ICM_20948_Stat_Ok)
        return ICM20948_ERR_COMM;
    result = ICM_20948_enable_DMP(&p_dev->c_dev, false);
    if (result != ICM_20948_Stat_Ok)
        return ICM20948_ERR_COMM;

    // Set Full Scale ranges
    ICM_20948_fss_t myFSS = {.a = gpm4, .g = dps2000};
    result = ICM_20948_set_full_scale(&p_dev->c_dev, (ICM_20948_Internal_Acc | ICM_20948_Internal_Gyr), myFSS);
    if (result != ICM_20948_Stat_Ok)
        return ICM20948_ERR_COMM;

    // Enable Gyro DLPF
    result = ICM_20948_enable_dlpf(&p_dev->c_dev, ICM_20948_Internal_Gyr, true);
    if (result != ICM_20948_Stat_Ok)
        return ICM20948_ERR_COMM;

    // Set sample rates
    ICM_20948_smplrt_t mySmplrt = {.g = 19, .a = 19}; // 55Hz Gyro, 56.25Hz Accel
    result = ICM_20948_set_sample_rate(&p_dev->c_dev, (ICM_20948_Internal_Acc | ICM_20948_Internal_Gyr), mySmplrt);
    if (result != ICM_20948_Stat_Ok)
        return ICM20948_ERR_COMM;

    // Reset and then load the DMP firmware
    result = ICM_20948_reset_FIFO(&p_dev->c_dev);
    if (result != ICM_20948_Stat_Ok)
        return ICM20948_ERR_COMM;
    result = ICM_20948_set_dmp_start_address(&p_dev->c_dev, DMP_START_ADDRESS);
    if (result != ICM_20948_Stat_Ok)
        return ICM20948_ERR_COMM;
    result = ICM_20948_firmware_load(&p_dev->c_dev);
    if (result != ICM_20948_Stat_Ok)
        return ICM20948_ERR_COMM;
    result = ICM_20948_set_dmp_start_address(&p_dev->c_dev, DMP_START_ADDRESS);
    if (result != ICM_20948_Stat_Ok)
        return ICM20948_ERR_COMM;

    // Set Hardware Fix Disable register
    result = ICM_20948_set_bank(&p_dev->c_dev, 0);
    if (result != ICM_20948_Stat_Ok)
        return ICM20948_ERR_COMM;
    uint8_t fix = 0x48;
    result = ICM_20948_execute_w(&p_dev->c_dev, AGB0_REG_HW_FIX_DISABLE, &fix, 1);
    if (result != ICM_20948_Stat_Ok)
        return ICM20948_ERR_COMM;

    // Set Single FIFO Priority Select register
    uint8_t fifo_prio = 0xE4;
    result = ICM_20948_execute_w(&p_dev->c_dev, AGB0_REG_SINGLE_FIFO_PRIORITY_SEL, &fifo_prio, 1);
    if (result != ICM_20948_Stat_Ok)
        return ICM20948_ERR_COMM;

    // Configure Accel scaling to DMP
    const unsigned char acc_scale[4] = {0x04, 0x00, 0x00, 0x00};
    result = (ICM_20948_Status_e)inv_icm20948_write_mems(&p_dev->c_dev, ACC_SCALE, 4, &acc_scale[0]);
    if (result != ICM_20948_Stat_Ok)
        return ICM20948_ERR_COMM;

    // Configure Gyro SF
    result = (ICM_20948_Status_e)inv_icm20948_set_gyro_sf(&p_dev->c_dev, 19, 3);
    if (result != ICM_20948_Stat_Ok)
        return ICM20948_ERR_COMM;

    PLATFORM_LOG_INFO("DMP Initialized Successfully.");
    return ICM20948_OK;
}

/*********************************************/
/* Static Functions                */
/*********************************************/

static icm20948_err_t icm20948_write_reg(icm20948_dev_t *p_dev, uint8_t reg, const uint8_t *p_data, uint32_t len)
{
    if (p_dev == NULL || p_dev->comm.write_reg == NULL)
        return ICM20948_ERR_INVALID_PARAM;
    if (p_dev->comm.write_reg(&p_dev->comm.bus_cfg, reg, p_data, len, &p_dev->comm.comm_cfg) != PLATFORM_HAL_OK)
    {
        return ICM20948_ERR_COMM;
    }
    return ICM20948_OK;
}

static icm20948_err_t icm20948_read_reg(icm20948_dev_t *p_dev, uint8_t reg, uint8_t *p_data, uint32_t len)
{
    if (p_dev == NULL || p_dev->comm.read_reg == NULL)
        return ICM20948_ERR_INVALID_PARAM;
    if (p_dev->comm.read_reg(&p_dev->comm.bus_cfg, reg, p_data, len, &p_dev->comm.comm_cfg) != PLATFORM_HAL_OK)
    {
        return ICM20948_ERR_COMM;
    }
    return ICM20948_OK;
}

static icm20948_err_t icm20948_mag_write_reg(icm20948_dev_t *p_dev, uint8_t reg, uint8_t data)
{
    if (ICM_20948_i2c_master_single_w(&p_dev->c_dev, MAG_AK09916_I2C_ADDR, reg, &data) != ICM_20948_Stat_Ok)
    {
        return ICM20948_ERR_COMM;
    }
    return ICM20948_OK;
}

static icm20948_err_t icm20948_mag_read_reg(icm20948_dev_t *p_dev, uint8_t reg, uint8_t *p_data)
{
    if (ICM_20948_i2c_master_single_r(&p_dev->c_dev, MAG_AK09916_I2C_ADDR, reg, p_data) != ICM_20948_Stat_Ok)
    {
        return ICM20948_ERR_COMM;
    }
    return ICM20948_OK;
}
icm20948_err_t icm20948_enable_dmp_sensor(icm20948_dev_t *p_dev, enum inv_icm20948_sensor sensor, bool enable)
{
    ICM_20948_Status_e status = inv_icm20948_enable_dmp_sensor(&p_dev->c_dev, sensor, enable ? 1 : 0);
    return (status == ICM_20948_Stat_Ok) ? ICM20948_OK : ICM20948_ERR_COMM; // Or more specific error mapping if needed
}

icm20948_err_t icm20948_set_dmp_odr_rate(icm20948_dev_t *p_dev, enum DMP_ODR_Registers odr_reg, uint16_t interval)
{
    ICM_20948_Status_e status = inv_icm20948_set_dmp_sensor_period(&p_dev->c_dev, odr_reg, interval);
    return (status == ICM_20948_Stat_Ok) ? ICM20948_OK : ICM20948_ERR_COMM;
}

icm20948_err_t icm20948_enable_fifo(icm20948_dev_t *p_dev, bool enable)
{
    ICM_20948_Status_e status = ICM_20948_enable_FIFO(&p_dev->c_dev, enable);
    return (status == ICM_20948_Stat_Ok) ? ICM20948_OK : ICM20948_ERR_COMM;
}

icm20948_err_t icm20948_enable_dmp(icm20948_dev_t *p_dev, bool enable)
{
    ICM_20948_Status_e status = ICM_20948_enable_DMP(&p_dev->c_dev, enable);
    return (status == ICM_20948_Stat_Ok) ? ICM20948_OK : ICM20948_ERR_COMM;
}

icm20948_err_t icm20948_reset_dmp(icm20948_dev_t *p_dev)
{
    ICM_20948_Status_e status = ICM_20948_reset_DMP(&p_dev->c_dev);
    return (status == ICM_20948_Stat_Ok) ? ICM20948_OK : ICM20948_ERR_COMM;
}

icm20948_err_t icm20948_reset_fifo(icm20948_dev_t *p_dev)
{
    ICM_20948_Status_e status = ICM_20948_reset_FIFO(&p_dev->c_dev);
    return (status == ICM_20948_Stat_Ok) ? ICM20948_OK : ICM20948_ERR_COMM;
}

icm20948_err_t icm20948_read_dmp_data_from_fifo(icm20948_dev_t *p_dev, icm_20948_DMP_data_t *data,
                                                ICM_20948_Status_e *p_status)
{
    if (p_dev == NULL || data == NULL || p_status == NULL)
    {
        return ICM20948_ERR_INVALID_PARAM;
    }

    *p_status = inv_icm20948_read_dmp_data(&p_dev->c_dev, data);

    if ((*p_status == ICM_20948_Stat_Ok) || (*p_status == ICM_20948_Stat_FIFOMoreDataAvail))
    {
        return ICM20948_OK; // Valid data was read
    }
    else if (*p_status == ICM_20948_Stat_FIFONoDataAvail)
    {
        return ICM20948_ERR_NO_DATA; // No data available is not necessarily a communication error
    }
    else
    {
        return ICM20948_ERR_COMM; // Other statuses indicate a problem
    }
}