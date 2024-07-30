/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * SPDX-FileCopyrightText: 2018 Manuel Weichselbaumer <mincequi@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "a2dp-codecs.h"

/* Currently our capabilities are limited to the most common use cases.
   iOS has a fixed stream configuration anyway (for SBC and AAC). */
const a2dp_sbc_t sbcCapabilities = {
    .frequency =
    /*SBC_SAMPLING_FREQ_16000 |
    SBC_SAMPLING_FREQ_32000 |*/
    SBC_SAMPLING_FREQ_44100 |
    SBC_SAMPLING_FREQ_48000,
    .channel_mode =
    /*SBC_CHANNEL_MODE_MONO |
    SBC_CHANNEL_MODE_DUAL_CHANNEL |*/
    SBC_CHANNEL_MODE_STEREO |
    SBC_CHANNEL_MODE_JOINT_STEREO,
    .block_length =
    SBC_BLOCK_LENGTH_4 |
    SBC_BLOCK_LENGTH_8 |
    SBC_BLOCK_LENGTH_12 |
    SBC_BLOCK_LENGTH_16,
    .subbands =
    SBC_SUBBANDS_4 |
    SBC_SUBBANDS_8,
    .allocation_method =
    SBC_ALLOCATION_SNR |
    SBC_ALLOCATION_LOUDNESS,
    .min_bitpool = MIN_BITPOOL,
    .max_bitpool = MAX_BITPOOL,
};

const a2dp_aac_t aacCapabilities = {
    .object_type =
    AAC_OBJECT_TYPE_MPEG2_AAC_LC |
    AAC_OBJECT_TYPE_MPEG4_AAC_LC,
    /*AAC_OBJECT_TYPE_MPEG4_AAC_LTP |
    AAC_OBJECT_TYPE_MPEG4_AAC_SCA, */
    AAC_INIT_FREQUENCY(
    /*AAC_SAMPLING_FREQ_8000 |
    AAC_SAMPLING_FREQ_11025 |
    AAC_SAMPLING_FREQ_12000 |
    AAC_SAMPLING_FREQ_16000 |
    AAC_SAMPLING_FREQ_22050 |
    AAC_SAMPLING_FREQ_24000 |
    AAC_SAMPLING_FREQ_32000 |*/
    AAC_SAMPLING_FREQ_44100 |
    AAC_SAMPLING_FREQ_48000)
    /*AAC_SAMPLING_FREQ_64000 |
    AAC_SAMPLING_FREQ_88200 |
    AAC_SAMPLING_FREQ_96000)*/
    .channels =
    /*AAC_CHANNELS_1 |*/
    AAC_CHANNELS_2,
    .vbr = 1,
    AAC_INIT_BITRATE(0xFFFF)
};
