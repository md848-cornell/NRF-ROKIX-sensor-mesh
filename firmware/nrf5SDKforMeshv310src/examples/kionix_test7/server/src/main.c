/* Copyright (c) 2010 - 2018, Nordic Semiconductor ASA
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
*
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdint.h>
#include <string.h>

/* HAL */
#include "boards.h"
//#include "simple_hal.h"
#include "app_timer.h"

/* Core */
#include "nrf_mesh_config_core.h"
#include "nrf_mesh_gatt.h"
#include "nrf_mesh_configure.h"
#include "nrf_mesh.h"
#include "mesh_stack.h"
#include "device_state_manager.h"
#include "access_config.h"
#include "proxy.h"

/* Provisioning and configuration */
#include "mesh_provisionee.h"
#include "mesh_app_utils.h"

/* Models */
#include "bsense_server.h"
#include "bsense_opcodes.h"
#include "bsense_messages.h"
#include "bsense_common.h"

/* Logging and RTT */
#include "log.h"
#include "rtt_input.h"

/* Example specific includes */
#include "app_config.h"
#include "example_common.h"
#include "nrf_mesh_config_examples.h"
#include "light_switch_example_common.h"
#include "ble_softdevice_support.h"


// NRF logging
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

//#include "kionix_main.c"

///////////////////////////////////////
///////////////////////////////////////
///////////////////////////////////////
///////////////////////////////////////

#include "nordic_common.h"
#include "nrf.h"
#include "bsp.h"

#include "app_uart.h"
#include "app_util_platform.h"
#include "app_scheduler.h"


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
    NRF_LOG_INFO("clock error code: %d", err_code)
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
    //power_manage();
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


void application_event_handler(void * p_event_data, uint16_t event_size)
{
    uint8_t size;
    ret_code_t ret;
    uint8_t data[MAX_SENSOR_DATA_SIZE];

    app_internal_evt_t *event = (app_internal_evt_t*)p_event_data;
    if(event->type == EVENT_READ_TWI) {
        ret = event->app_events.sensor_evt.sensor_read(data, &size);
        if (ret == NRF_SUCCESS) {            
            /* Add sensor ID check here for passing certain sensor data to USB or BLE*/
            if(event->app_events.sensor_evt.sensor_id == KX122_ID) {
                //ble_uart_tx(data, size);
                //usb_serial_tx(data, size);
            }
        }
    } else if(event->type == EVENT_BATTERY_MEASUREMENT) {
        //NRF_LOG_INFO( "ADC raw=%d", event->app_events.adc_evt.value);
        //NRF_LOG_INFO( "ADC=%d mV", ADC_RESULT_IN_MILLI_VOLTS(event->app_events.adc_evt.value));
        /*Add code here to send battery measurement result over USB or BLE*/
    }

    
}

///////////////////////////////////////////
///////////////////////////////////////
///////////////////////////////////////
///////////////////////////////////////





















#define BSENSE_LED          (BSP_LED_0)
#define app_bsense_ELEMENT_INDEX     (0)

static bool m_device_provisioned;

static bsense_server_t m_bsense_server;

/*************************************************************************************************/

/*************************************************************************************************/

static void node_reset(void)
{
    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "----- Node reset  -----\n");
    //hal_led_blink_ms(LEDS_MASK, LED_BLINK_INTERVAL_MS, LED_BLINK_CNT_RESET);
    /* This function may return if there are ongoing flash operations. */
    mesh_stack_device_reset();
}

static void config_server_evt_cb(const config_server_evt_t * p_evt)
{
    if (p_evt->type == CONFIG_SERVER_EVT_NODE_RESET)
    {
        node_reset();
    }
}

static uint8_t button_press_counter;

static void button_event_handler(uint32_t button_number)
{
    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Button %u pressed\n", button_number);
    switch (button_number)
    {
        /* Pressing SW1 on the Development Kit will result in LED state to toggle and trigger
        the STATUS message to inform client about the state change. This is a demonstration of
        state change publication due to local event. */
        case 0:
        {
            //uint8_t bsense_button_state;
            /*bsense_button_state = get_m_bsense_button_state();
            if (bsense_button_state == 1) 
            {
            bsense_button_state = 0;
            }
            else 
            {
            bsense_button_state = 1;
            }*/
            //bsense_button_state = button_press_counter;
            //set_m_bsense_button_state(bsense_button_state);
            //__LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "button sensor state: %d\n", get_m_bsense_button_state());
            //hal_led_pin_set(BSENSE_LED, get_m_bsense_button_state());
            //button_press_counter++;
            break;
        }

        /* Initiate node reset */
        case 3:
        {
            /* Clear all the states to reset the node. */
            if (mesh_stack_is_device_provisioned())
            {
#if MESH_FEATURE_GATT_PROXY_ENABLED
                (void) proxy_stop();
#endif
                mesh_stack_config_clear();
                node_reset();            }
            else
            {
                __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "The device is unprovisioned. Resetting has no effect.\n");
            }
            break;
        }

        default:
            break;
    }
}

static void app_rtt_input_handler(int key)
{
    if (key >= '0' && key <= '4')
    {
        uint32_t button_number = key - '0';
        button_event_handler(button_number);
    }
}

static void device_identification_start_cb(uint8_t attention_duration_s)
{
    //hal_led_mask_set(LEDS_MASK, false);
    //hal_led_blink_ms(BSP_LED_2_MASK  | BSP_LED_3_MASK,
    //                 LED_BLINK_ATTENTION_INTERVAL_MS,
    //                 LED_BLINK_ATTENTION_COUNT(attention_duration_s));
}

static void provisioning_aborted_cb(void)
{
    NRF_LOG_INFO("Provisioning aborted\n");
    //hal_led_blink_stop();
}

static void provisioning_complete_cb(void)
{
    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Successfully provisioned\n");
    NRF_LOG_INFO("Successfully provisioned\n");

#if MESH_FEATURE_GATT_ENABLED
    /* Restores the application parameters after switching from the Provisioning
     * service to the Proxy  */
    gap_params_init();
    conn_params_init();
#endif

    dsm_local_unicast_address_t node_address;
    dsm_local_unicast_addresses_get(&node_address);
    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Node Address: 0x%04x \n", node_address.address_start);
    NRF_LOG_INFO("Node Address: 0x%04x \n", node_address.address_start);
    //hal_led_blink_stop();
    //hal_led_mask_set(LEDS_MASK, LED_MASK_STATE_OFF);
    //hal_led_blink_ms(LEDS_MASK, LED_BLINK_INTERVAL_MS, LED_BLINK_CNT_PROV);
}


bsense_server_attention_cb_t bsense_server_attention_cb(void)
{
    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "BSENSE server attention callback \n");
    uint16_t opcode = bsense_OPCODE_ATTENTION_STATUS;
    bsense_server_t * p_server = &m_bsense_server;
    uint8_t message_buffer[sizeof(bsense_msg_data_status_t) + bsense_SERVER_data_ARRAY_SIZE];
    bsense_msg_data_status_t * p_pdu = (bsense_msg_data_status_t *) message_buffer;

    p_pdu->test_id = p_server->previous_test_id;
    p_pdu->company_id = p_server->company_id;

    /* Create the list of datas from the data array bitfield: */
    uint8_t current_index = 0;
    //for (uint32_t i = bitfield_next_get(*p_data_array, bsense_SERVER_DATA_ARRAY_SIZE, 0); i != bsense_SERVER_DATA_ARRAY_SIZE;
    //        i = bitfield_next_get(*p_data_array, bsense_SERVER_DATA_ARRAY_SIZE, i + 1))
    //{
    uint8_t button_state = get_m_bsense_button_state();
    //__LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "SENDING button sensor state set: %d\n", button_state);
    p_pdu->data_array[current_index] = button_state;
    current_index++;
    //}

    const access_message_tx_t packet =
    {
        .opcode = ACCESS_OPCODE_SIG(opcode),
        .p_buffer = message_buffer,
        .length = ((uintptr_t) &p_pdu->data_array[current_index] - (uintptr_t) &message_buffer[0]),
        .force_segmented = false,
        .transmic_size = NRF_MESH_TRANSMIC_SIZE_DEFAULT,
        .access_token = nrf_mesh_unique_token_get()
    }; /*lint !e446: side effect in initializer */

    (void) access_model_publish(p_server->model_handle, &packet);
}

bsense_server_t * mesh_stack_bsense_server_get(void)
{
    return &m_bsense_server;
}

static void models_init_cb(void)
{
    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Initializing and adding models\n");

    /* Initialize the bsense server for the primary element */
    uint32_t status;
    status = bsense_server_init(&m_bsense_server, 0, DEVICE_COMPANY_ID,
                                &bsense_server_attention_cb);
    if (status != NRF_SUCCESS)
    {
        __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Error initalizing bsense: %d\n", status);
    }

    //app_model_init();
}

static void mesh_init(void)
{
    mesh_stack_init_params_t init_params =
    {
        .core.irq_priority       = NRF_MESH_IRQ_PRIORITY_LOWEST,
        .core.lfclksrc           = DEV_BOARD_LF_CLK_CFG,
        .core.p_uuid             = NULL,
        .models.models_init_cb   = models_init_cb,
        .models.config_server_cb = config_server_evt_cb
    };
    // health_server_init happens in here
    ERROR_CHECK(mesh_stack_init(&init_params, &m_device_provisioned));

}

static void mk_provisioned_mesh_node_reset(void)
{
    if (mesh_stack_is_device_provisioned())
    {
        NRF_LOG_INFO("NODE WAS PROVISIONED - RESETTING");
        // turned off proxy thing because everything breaks
        if (0)
        {
            #if MESH_FEATURE_GATT_PROXY_ENABLED
                              (void) proxy_stop();
            #endif
         }
         mesh_stack_config_clear();
         node_reset();  
         NRF_LOG_INFO("FINISHED RESETTING PROVISIONED NODE");
    } else {
        NRF_LOG_INFO("NODE WAS NOT PROVISIONED");
    }
}

static void initialize(void)
{
    __LOG_INIT(LOG_SRC_APP | LOG_SRC_ACCESS | LOG_SRC_BEARER, LOG_LEVEL_INFO, LOG_CALLBACK_DEFAULT);
    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "----- BLE Mesh Light Switch Server Demo -----\n");

    ERROR_CHECK(app_timer_init());
    //hal_leds_init();

//#if BUTTON_BOARD
   // ERROR_CHECK(hal_buttons_init(button_event_handler));
//#endif

    ble_stack_init();

#if MESH_FEATURE_GATT_ENABLED
    gap_params_init();
    conn_params_init();
#endif

    mesh_init();
}

static void start(void)
{
    rtt_input_enable(app_rtt_input_handler, RTT_INPUT_POLL_PERIOD_MS);

    if (!m_device_provisioned)
    {
        static const uint8_t static_auth_data[NRF_MESH_KEY_SIZE] = STATIC_AUTH_DATA;
        mesh_provisionee_start_params_t prov_start_params =
        {
            .p_static_data    = static_auth_data,
            .prov_complete_cb = provisioning_complete_cb,
            .prov_device_identification_start_cb = device_identification_start_cb,
            .prov_device_identification_stop_cb = NULL,
            .prov_abort_cb = provisioning_aborted_cb,
            .p_device_uri = EX_URI_LS_SERVER
        };
        ERROR_CHECK(mesh_provisionee_prov_start(&prov_start_params));
    }

    mesh_app_uuid_print(nrf_mesh_configure_device_uuid_get());

    ERROR_CHECK(mesh_stack_start());

    //hal_led_mask_set(LEDS_MASK, LED_MASK_STATE_OFF);
    //hal_led_blink_ms(LEDS_MASK, LED_BLINK_INTERVAL_MS, LED_BLINK_CNT_START);
}


// Kionix initialization

static void kionix_init(void)
{
    NRF_LOG_INFO( "## kionix init started");
    
    /*Board init, gpio default values are set.*/
    sensor_node_board_init();
    NRF_LOG_INFO( "sensor board");
    /*Init platform peripherals*/
    nrf52_timers_init();
    NRF_LOG_INFO( "timers");
    //nrf52_clock_init();
    nrf52_gpiote_init();
    NRF_LOG_INFO( "gpiote");
    
    /*Init leds.*/
    //buttons_leds_init();
    
    /*Init scheduler, context swich from ISR to main.*/
    scheduler_init();
    NRF_LOG_INFO( "scheduler");

    /*Input pin configure example.*/
    configure_gpio_as_input_example();
    /*Output pin configure example.*/
    configure_gpio_as_output_example();
    NRF_LOG_INFO( "gpio configured");
    
    /*Init USB serial and BLE uart.*/
    //usb_serial_init();
    //ble_uart_init();
    //usbd_serial_power_events_enable();
    
    nrf52_twi_init();
    NRF_LOG_INFO( "twi");
    /*Sensor driver initialize, default KX122.*/
    nrf52_platform_functions_init();
    NRF_LOG_INFO( "platform functions");
    sensors_init(&nrf52_funcs);
    NRF_LOG_INFO( "sensors");

    /*Init and start battery measurement example.*/
    battery_measurement_init();
    battery_measurement_start();
    NRF_LOG_INFO( "## kionix init finished");
}










/**@brief Function for initializing the nrf_log module.*/
static void knrf_log_init(void)
{
    ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);

    NRF_LOG_DEFAULT_BACKENDS_INIT();
}
#include "KX122_drv.h"
static void sensor_read_test_to_log(void)
{
    NRF_LOG_INFO("### READING SENSOR DATA ###");
    uint8_t size;
    ret_code_t ret;
    uint8_t data[MAX_SENSOR_DATA_SIZE];

    ret = KX122_get_raw_xyz(data, &size);
    NRF_LOG_INFO("Read return value: %d", ret);
    int16_t v1;
    int16_t v2;
    int16_t v3;
    v1 = ((int16_t) data[1] << 8) | (int16_t)data[0];
    v2 = ((int16_t) data[3] << 8) | (int16_t)data[2];
    v3 = ((int16_t) data[5] << 8) | (int16_t)data[4];

    NRF_LOG_INFO("########################## values: %d, %d, %d", v1, v2, v3);
}


int main(void)
{
    knrf_log_init();
    NRF_LOG_INFO("### LOGS INITIALIZED ###");
    NRF_LOG_INFO("### starting general initialization ###");

    kionix_init();
    NRF_LOG_INFO("### finished kionix initialization ###");

    initialize();
    NRF_LOG_INFO("### finished initalization ###");

    start();
    NRF_LOG_INFO("### finished startup sequence ###");
    
    
    // reset the node
    mk_provisioned_mesh_node_reset();

    for (;;)
    {
        (void)sd_app_evt_wait();
        //while (app_usbd_event_queue_process())
        //{
        //    /* Nothing to do */
        //}
        app_sched_execute();
        idle_state_handle();
    }
}
