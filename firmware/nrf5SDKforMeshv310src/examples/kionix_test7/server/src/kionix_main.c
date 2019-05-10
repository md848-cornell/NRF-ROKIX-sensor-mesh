
#ifndef KIONIX_MAIN__C_
#define KIONIX_MAIN__C_
/////////////// from kionix
#include <stdint.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf.h"
#include "boards.h"
#include "bsp.h"
#include "app_timer.h"
#include "app_uart.h"
#include "app_util_platform.h"
#include "app_scheduler.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "nrf_gpio.h"
#include "nrf_drv_clock.h"
#include "nrf_delay.h"
#include "nrf_drv_power.h"
#include "app_usbd.h"

//#include "usb_serial.h"
//#include "ble_uart.h"
#include "sensor_node_initialize.h"
#include "platform_functions.h"
#include "sensors.h"
#include "battery_measurement.h"

#define ADC_REF_VOLTAGE_IN_MILLIVOLTS  600  //!< Reference voltage (in milli volts) used by ADC while doing conversion.
#define ADC_RES_12BIT                  4096 //!< Maximum digital value for 12-bit ADC conversion.
#define ADC_PRE_SCALING_COMPENSATION   8    //!< The ADC is configured to use VDD with 1/4 prescaling as input. And hence the result of conversion is to be multiplied by 4 to get the actual value of the battery voltage.

#define ADC_RESULT_IN_MILLI_VOLTS(ADC_VALUE) \
    ((((ADC_VALUE) *ADC_REF_VOLTAGE_IN_MILLIVOLTS) / ADC_RES_12BIT) * ADC_PRE_SCALING_COMPENSATION)

/**@brief Platform functions used by the sensor driver(s).
 *
 * @details Entry point for the platform specific features such as TWI.
 */
struct platform_functions nrf52_funcs;


/**@brief Defines for the application scheduler.
 *
 * @details Application scheduler is used to change execution from the ISR context to main.
 */
#define SCHED_MAX_EVENT_DATA_SIZE      (sizeof(app_internal_evt_t))
#define SCHED_QUEUE_SIZE               20


/**@brief Function for initializing the application event scheduler.*/
static void scheduler_init(void)
{
    APP_SCHED_INIT(SCHED_MAX_EVENT_DATA_SIZE, SCHED_QUEUE_SIZE);
}




/**@brief Set Sensor node GPIOs to their default position.*/
void sensor_node_board_init(void)
{
    battery_control_pins_init();
    sensor_int_pins_init();
    external_radio_power_control_pins_init();
    nfc_pins_init();
    swd_signal_init();
    general_io_pins_init();
    twi_spi_pins_init();
    analog_input_pins_init();
    uarte_input_pins_init();
    power_control_init();
}

/**@brief Initialize clock driver.*/
void nrf52_clock_init(void) {

    ret_code_t err_code;
    err_code = nrf_drv_clock_init();
    APP_ERROR_CHECK(err_code);
}

/**@brief Function poiters to platform spesific functionality*/
void nrf52_platform_functions_init(void) {
  
    nrf52_funcs.twi_write     = nrf52_twi_write;
    nrf52_funcs.twi_read      = nrf52_twi_read;    
    nrf52_funcs.delay_ms      = nrf52_delay_ms;
    nrf52_funcs.debug_println = nrf52_debug_println;
}



/** @brief Function for initializing the timer module. */
static void nrf52_timers_init(void)
{
    ret_code_t err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);
}


/** @brief Function for initializing buttons and LEDs. */
static void buttons_leds_init(void)
{
    uint32_t err_code = bsp_init(BSP_INIT_LEDS, NULL);
    APP_ERROR_CHECK(err_code);
}


/**
 * @brief Function for handling the idle state (main loop).
 *
 * @details If there is no pending log operation, then sleep until next the next event occurs.
 */
static void idle_state_handle(void)
{
    UNUSED_RETURN_VALUE(NRF_LOG_PROCESS());
    power_manage();
}


/**
 * @brief Output pin configure example.
 *
 * @details This is just an example which can modified for own purpose.
 */
void configure_gpio_as_output_example()
{
    
    /*LED_R*/
    uint8_t pin = NRF_GPIO_PIN_MAP(1,5);

    /*Configure pin as output*/
    nrf_gpio_cfg(
        pin,
        NRF_GPIO_PIN_DIR_OUTPUT,
        /* This will keep input buffer connected,
         * so that pin changes are also seen in input register. */
        NRF_GPIO_PIN_INPUT_CONNECT,
        NRF_GPIO_PIN_NOPULL,
        NRF_GPIO_PIN_S0S1,
        NRF_GPIO_PIN_NOSENSE);
    
    for(int i= 0; i < 5; i++) {
        /*Drive pin to high.*/
        nrf_gpio_pin_set(pin);
        /* Wait 100 ms. */
        nrf_delay_ms(100);
        /*Drive pin to low.*/
        nrf_gpio_pin_clear(pin);
        /* Wait 100 ms. */
        nrf_delay_ms(100);
    }
}


/**
 * @brief Input pin configure and read example.
 *
 * @details This is just an example which can modified for own purpose.
 */
void configure_gpio_as_input_example()
{
    uint32_t state = 0;
    uint8_t pin = NRF_GPIO_PIN_MAP(0,16); /*INT_R, Trigger, sync, interrupt and board detection signals*/

    nrf_gpio_pin_pull_t pull_config = NRF_GPIO_PIN_NOPULL;
    nrf_gpio_cfg_input(pin, pull_config);

    state = nrf_gpio_pin_read(pin);
    NRF_LOG_INFO("Pin state=%d", state);
}

#endif