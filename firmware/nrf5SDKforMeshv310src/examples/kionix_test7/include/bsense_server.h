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

#ifndef bsense_SERVER_H__
#define bsense_SERVER_H__

#include <stdbool.h>
#include <stdint.h>

#include "access.h"
#include "bitfield.h"

#include "bsense_common.h"
#include "nrf_mesh_config_app.h"

/**
 * @defgroup bsense_SERVER bsense Server
 * @ingroup bsense_MODEL
 * Model implementing the bsense Server foundation model.
 * @{
 */
//#include "log.h"

static uint8_t m_bsense_button_state;
static uint8_t bsense_message_number;

void set_m_bsense_button_state(uint8_t state);

uint8_t get_m_bsense_button_state(void);

/** Size of the bsense server data array. */
#define bsense_SERVER_data_ARRAY_SIZE  8

/** Object type for bsense server instances. */
typedef struct __bsense_server_t bsense_server_t;

/**
 * Callback function for the attention state.
 * The attention state is enabled when a bsense client sets the attention timer to a nonzero
 * value, and disabled when the attention timer reaches zero.
 * @param[in] p_server        Pointer to the bsense server instance structure.
 * @param[in] attention_state @c true if the device should enable the attention state, @c false
 *                            if the device should disable the attention state.
 */
typedef void (*bsense_server_attention_cb_t)(const bsense_server_t * p_server, bool attention_state);

/**
 * Self-test function type.
 *
 * A self-test should perform a specific test and set or clear corresponding data(s) in the data array
 * using the bsense_server_data_register() or bsense_server_data_clear() functions.
 *
 * @param[in] p_server   Pointer to the server instance structure.
 * @param[in] company_id Company ID of the requested test.
 * @param[in] test_id    ID of the test that should be run.
 *
 * @see bsense_server_data_register(), bsense_server_data_clear(), bsense_server_data_is_set()
 */
typedef void (*bsense_server_selftest_cb_t)(bsense_server_t * p_server, uint16_t company_id, uint8_t test_id);

/**
 * Structure defining a self-test function.
 * An array of these structs should be passed to the bsense server on initialization if self-tests are
 * supported by the device.
 */
typedef struct
{
    uint8_t                     test_id;            /**< Self-test ID. */
    bsense_server_selftest_cb_t selftest_function;  /**< Pointer to the self-test function. */
} bsense_server_selftest_t;

/** bsense server data array type. */
typedef uint32_t bsense_server_data_array_t[BITFIELD_BLOCK_COUNT(bsense_SERVER_data_ARRAY_SIZE)];

/**
 * bsense server instance structure.
 */
struct __bsense_server_t
{
    access_model_handle_t            model_handle;          /**< Model handle. */
    uint8_t                          fast_period_divisor;   /**< Fast period divisor, used to increase publishing interval when datas are present. */
    access_publish_resolution_t      regular_publish_res;   /**< Regular publication interval resolution. */
    uint8_t                          regular_publish_steps; /**< Regular publication interval steps. */
    uint8_t                          previous_test_id;      /**< ID of the latest self-test run by the model. */
    uint16_t                         company_id;            /**< bsense server company ID. */
    bsense_server_attention_cb_t     attention_handler;     /**< Handler for the attention state. If @c NULL, the attention state is unsupported. */
    uint8_t                          attention_timer;       /**< Timer for the attention state. */
    struct __bsense_server_t *       p_next;                /**< Pointer to the next instance. Used internally for supporting the attention timer. */
};

/**
 * Registers a data in the current data array.
 *
 * If the device is publishing a periodic Current Status message and there are no active datas
 * _before_ this function is called, the model will enable fast publishing of the Current Status
 * message. This is done by dividing the current publishing interval with 2<sup>fast period interval</sup>
 * and using the result as the new publishing interval.
 *
 * @note The fast publishing interval will never go lower than 100 ms.
 *
 * @param[in,out] p_server   Pointer to a server instance.
 * @param[in]     data_code ID of the data to register.
 *
 * @see bsense_server_data_clear()
 */
void bsense_server_data_register(bsense_server_t * p_server, uint8_t data_code);

/**
 * Clears a data from the current data array.
 *
 * If the device is currently publishing a periodic Current Status message with a fast publishing
 * period, and there are no active datas _after_ this function is called, the publishing interval
 * will be reset back to the original interval.
 *
 * @param[in,out] p_server   Pointer to a server instance.
 * @param[in]     data_code ID of the data to clear.
 *
 * @see bsense_server_data_register()
 */
void bsense_server_data_clear(bsense_server_t * p_server, uint8_t data_code);

/**
 * Checks if a data code is set in current data array.
 * @param[in,out] p_server   Pointer to a server instance.
 * @param[in]     data_code ID of the data to check for.
 * @return Returns @c true if the specified data is set and @c false otherwise.
 */
bool bsense_server_data_is_set(bsense_server_t * p_server, uint8_t data_code);

/**
 * Gets the number of currently set datas in the data array.
 * @param[in] p_server Pointer to a server instance.
 * @return Returns the number of currently set datas in the data array.
 */
uint8_t bsense_server_data_count_get(const bsense_server_t * p_server);

/**
 * Gets the current value of the attention timer.
 * @param[in] p_server Pointer to a server instance.
 * @return Returns the current value of the attention timer.
 */
uint8_t bsense_server_attention_get(const bsense_server_t * p_server);

/**
 * Sets the attention timer value.
 * @param[in] p_server  Pointer to a server instance.
 * @param[in] attention New value for the attention timer.
 */
void bsense_server_attention_set(bsense_server_t * p_server, uint8_t attention);

/**
 * Initializes the bsense server model.
 *
 * @param[in] p_server      Pointer to the server instance structure.
 * @param[in] element_index Element index to use when registering the bsense server.
 * @param[in] company_id    Company ID supported by this bsense model server.
 * @param[in] attention_cb  Callback function for enabling or disabling the attention state for
 *                          the device. If @c NULL is passed as this argument, the attention state
 *                          is considered to be unsupported by the model, and the attention timer
 *                          is disabled.
 *
 * @retval NRF_SUCCESS The model was successfully initialized.
 *
 * @see access_model_add()
 */
uint32_t bsense_server_init(bsense_server_t * p_server, uint16_t element_index, uint16_t company_id,
        bsense_server_attention_cb_t attention_cb);

/** @} */

#endif

