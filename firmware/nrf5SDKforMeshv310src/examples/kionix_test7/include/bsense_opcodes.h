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

#ifndef bsense_OPCODES_H__
#define bsense_OPCODES_H__

/**
 * @defgroup bsense_OPCODES bsense model message opcodes
 * @ingroup bsense_MODEL
 * Opcodes used in bsense model messages.
 * @{
 */

/** bsense model opcodes. */
typedef enum
{
    /** Opcode for the "bsense Current Status" message. */
    bsense_OPCODE_CURRENT_STATUS        = 0xACF4,
    /** Opcode for the "bsense data Status" message. */
    bsense_OPCODE_data_STATUS          = 0xACF5,
    /** Opcode for the "bsense Attention Get" message. */
    bsense_OPCODE_ATTENTION_GET         = 0xACE4,
    /** Opcode for the "bsense Attention Set" message. */
    bsense_OPCODE_ATTENTION_SET         = 0xACE5,
    /** Opcode for the "bsense Attention Set Unacknowledged" message. */
    bsense_OPCODE_ATTENTION_SET_UNACKED = 0xACE6,
    /** Opcode for the "bsense Attention Status" message. */
    bsense_OPCODE_ATTENTION_STATUS      = 0xACE7,
    /** Opcode for the "bsense data Clear" message. */
    bsense_OPCODE_data_CLEAR           = 0xACEf,
    /** Opcode for the "bsense data Clear Unacknowledged" message. */
    bsense_OPCODE_data_CLEAR_UNACKED   = 0xAC30,
    /** Opcode for the "bsense data Get" message. */
    bsense_OPCODE_data_GET             = 0xAC31,
    /** Opcode for the "bsense data Test" message. */
    bsense_OPCODE_data_TEST            = 0xAC32,
    /** Opcode for the "bsense data Test Unacknowledged" message. */
    bsense_OPCODE_data_TEST_UNACKED    = 0xAC33,
    /** Opcode for the "bsense Period Get" message. */
    bsense_OPCODE_PERIOD_GET            = 0xAC34,
    /** Opcode for the "bsense Period Set" message. */
    bsense_OPCODE_PERIOD_SET            = 0xAC35,
    /** Opcode for the "bsense Period Set Unacknowledged" message. */
    bsense_OPCODE_PERIOD_SET_UNACKED    = 0xAC36,
    /** Opcode for the "bsense Period Status" message. */
    bsense_OPCODE_PERIOD_STATUS         = 0xAC37,
} bsense_opcode_t;

/** @} */

#endif

