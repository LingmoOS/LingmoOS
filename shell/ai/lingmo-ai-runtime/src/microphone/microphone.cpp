/*
 * Copyright 2024 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "microphone.h"

#include <stdio.h>
Microphone::Microphone(PaStreamCallback *callback) : streamCallback_(callback)
{
    initMicrophone();
}

Microphone::~Microphone()
{
    destroyMicrophone();
}

void Microphone::startMicrophone()
{
    if (audioStart_ == true)
    {
        return;
    }
    audioStart_ = true;

    err =
        Pa_OpenStream(&stream, &param, nullptr, /* &outputParameters, */
                      sample_rate,
                      0,         // frames per buffer
                      paClipOff, // we won't output out of range samples
                                 // so don't bother clipping them
                      streamCallback_, nullptr);

    if (err != paNoError)
    {
        fprintf(stderr, "portaudio error: %s[] , func : [%s], line : [%s]", Pa_GetErrorText(err), __FUNCTION__, __LINE__);
    }

    err = Pa_StartStream(stream);
    if (err != paNoError)
    {
        fprintf(stderr, "portaudio error: %s[] , func : [%s], line : [%s]", Pa_GetErrorText(err), __FUNCTION__, __LINE__);
    }
}

void Microphone::stopMicrophone()
{
    if (audioStart_ == false)
    {
        return;
    }
    audioStart_ = false;

    err = Pa_CloseStream(stream);
    if (err != paNoError)
    {
        fprintf(stderr, "portaudio error: %s[] , func : [%s], line : [%s]", Pa_GetErrorText(err), __FUNCTION__, __LINE__);
    }
}

void Microphone::initMicrophone()
{
    err = Pa_Initialize();
    if (err != paNoError)
    {
        fprintf(stderr, "portaudio error: %s[] , func : [%s], line : [%s]", Pa_GetErrorText(err), __FUNCTION__, __LINE__);
    }

    param.device = Pa_GetDefaultInputDevice();
    if (param.device == paNoDevice)
    {
        fprintf(stderr, "No default input device found!");
    }

    const PaDeviceInfo *info = Pa_GetDeviceInfo(param.device);
    fprintf(stderr, "Name : [%s]", info->name);

    param.channelCount = 1;
    param.sampleFormat = paFloat32;

    param.suggestedLatency = info->defaultLowInputLatency;
    param.hostApiSpecificStreamInfo = nullptr;
}

void Microphone::destroyMicrophone()
{
    err = Pa_Terminate();
    if (err != paNoError)
    {
        fprintf(stderr, "portaudio error: %s[] , func : [%s], line : [%s]", Pa_GetErrorText(err), __FUNCTION__, __LINE__);
    }
}