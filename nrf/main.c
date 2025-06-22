#include "app_error.h"
#include "app_util_platform.h"
#include "boards.h"
#include "nrf_delay.h"
#include "nrf_drv_spi.h"
#include "nrf_gpio.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "nrf_uarte.h"
#include <stdio.h>
#include <string.h>

#define SPI_INSTANCE 0

#define SPI_SS_PIN 28
#define SPI_MISO_PIN 29
#define SPI_SCK_PIN 30
#define SPI_MOSI_PIN 31

#define WHO_AM_I_REG 0x75
#define EXPECTED_WHO_AM_I 0xEA    // Change depending on the ICM chip used

static const nrf_drv_spi_t spi = NRF_DRV_SPI_INSTANCE(SPI_INSTANCE);
static volatile bool spi_xfer_done;

static uint8_t m_tx_buf[2];
static uint8_t m_rx_buf[2];

void spi_event_handler(nrf_drv_spi_evt_t const *p_event, void *p_context) {
  spi_xfer_done = true;
}

void read_icm_who_am_i(void) {
  spi_xfer_done = false;
  m_tx_buf[0]   = WHO_AM_I_REG | 0x80;    // Set MSB for read
  m_tx_buf[1]   = 0x00;

  memset(m_rx_buf, 0, sizeof(m_rx_buf));

  APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi, m_tx_buf, 2, m_rx_buf, 2));

  while (!spi_xfer_done) {
    __WFE();
  }

  NRF_LOG_INFO("WHO_AM_I = 0x%02X", m_rx_buf[1]);
  if (m_rx_buf[1] == EXPECTED_WHO_AM_I) {
    NRF_LOG_INFO("ICM WHO_AM_I correct!");
  } else {
    NRF_LOG_ERROR("ICM WHO_AM_I mismatch!");
  }
  NRF_LOG_FLUSH();
}

int main(void) {
  bsp_board_init(BSP_INIT_LEDS);
  APP_ERROR_CHECK(NRF_LOG_INIT(NULL));
  NRF_LOG_DEFAULT_BACKENDS_INIT();

  nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG;
  spi_config.ss_pin               = SPI_SS_PIN;
  spi_config.miso_pin             = SPI_MISO_PIN;
  spi_config.mosi_pin             = SPI_MOSI_PIN;
  spi_config.sck_pin              = SPI_SCK_PIN;

  APP_ERROR_CHECK(nrf_drv_spi_init(&spi, &spi_config, spi_event_handler, NULL));

  NRF_LOG_INFO("ICM WHO_AM_I check started.");

  while (1) {
    read_icm_who_am_i();
    bsp_board_led_invert(BSP_BOARD_LED_0);
    nrf_delay_ms(1000);
  }
}
