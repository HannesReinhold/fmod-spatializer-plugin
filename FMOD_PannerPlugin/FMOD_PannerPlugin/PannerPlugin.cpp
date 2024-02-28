//
//  DelayUnit.cpp
//  DelayPlugin
//
//  Created by James Kelly on 14/12/2018.
//  Copyright © 2018 James Kelly. All rights reserved.
//
//  This file is a 'blank' plugin that does not process audio and only lets it pass through
#include "pch.h"

#include <math.h>
#include <stdio.h>
#include <string>

#include "dsp_utils.h"

#include "fmod.hpp"
#include "FirstOrderLowpass.h"
#include "Delay.h"
#include "Biquad.h"
#include "Reverb.h"
#include "libBasicSOFA-master/libBasicSOFA/BasicSOFA/BasicSOFA/BasicSOFA.hpp"

extern "C"
{
    F_EXPORT FMOD_DSP_DESCRIPTION* F_CALL FMODGetDSPDescription();
}

// ==================== //
// CALLBACK DEFINITIONS //
// ==================== //
FMOD_RESULT Create_Callback(FMOD_DSP_STATE* dsp_state);
FMOD_RESULT Release_Callback(FMOD_DSP_STATE* dsp_state);
FMOD_RESULT Reset_Callback(FMOD_DSP_STATE* dsp_state);
FMOD_RESULT Read_Callback(FMOD_DSP_STATE* dsp_state, float* inbuffer, float* outbuffer, unsigned int length, int inchannels, int* outchannels);
FMOD_RESULT Process_Callback(FMOD_DSP_STATE* dsp_state, unsigned int length, const FMOD_DSP_BUFFER_ARRAY* inbufferarray, FMOD_DSP_BUFFER_ARRAY* outbufferarray, FMOD_BOOL inputsidle, FMOD_DSP_PROCESS_OPERATION op);
FMOD_RESULT SetPosition_Callback(FMOD_DSP_STATE* dsp_state, unsigned int pos);
FMOD_RESULT ShouldIProcess_Callback(FMOD_DSP_STATE* dsp_state, FMOD_BOOL inputsidle, unsigned int length, FMOD_CHANNELMASK inmask, int inchannels, FMOD_SPEAKERMODE speakermode);

FMOD_RESULT SetFloat_Callback(FMOD_DSP_STATE* dsp_state, int index, float value);
FMOD_RESULT SetInt_Callback(FMOD_DSP_STATE* dsp_state, int index, int value);
FMOD_RESULT SetBool_Callback(FMOD_DSP_STATE* dsp_state, int index, FMOD_BOOL value);
FMOD_RESULT SetData_Callback(FMOD_DSP_STATE* dsp_state, int index, void* data, unsigned int length);
FMOD_RESULT GetFloat_Callback(FMOD_DSP_STATE* dsp_state, int index, float* value, char* valuestr);
FMOD_RESULT GetInt_Callback(FMOD_DSP_STATE* dsp_state, int index, int* value, char* valuestr);
FMOD_RESULT GetBool_Callback(FMOD_DSP_STATE* dsp_state, int index, FMOD_BOOL* value, char* valuestr);
FMOD_RESULT GetData_Callback(FMOD_DSP_STATE* dsp_state, int index, void** data, unsigned int* length, char* valuestr);

FMOD_RESULT SystemRegister_Callback(FMOD_DSP_STATE* dsp_state);
FMOD_RESULT SystemDeregister_Callback(FMOD_DSP_STATE* dsp_state);
FMOD_RESULT SystemMix_Callback(FMOD_DSP_STATE* dsp_state, int stage);

// ==================== //
//      PARAMETERS      //
// ==================== //

static FMOD_DSP_PARAMETER_DESC p_gain;
static FMOD_DSP_PARAMETER_DESC p_pan;
static FMOD_DSP_PARAMETER_DESC p_cutoffFrequency;

FMOD_DSP_PARAMETER_DESC* PluginsParameters[3] =
{
    &p_gain,
    &p_pan,
    &p_cutoffFrequency
};

// ==================== //
//     SET CALLBACKS    //
// ==================== //

FMOD_DSP_DESCRIPTION PluginCallbacks =
{
    FMOD_PLUGIN_SDK_VERSION,    // version
    "Panner",          // name
    0x00010000,                 // plugin version
    1,                          // no. input buffers
    1,                          // no. output buffers
    Create_Callback,            // create
    Release_Callback,           // release
    Reset_Callback,             // reset
    Read_Callback,              // read
    Process_Callback,           // process
    SetPosition_Callback,       // setposition
    3,                          // no. parameter
    PluginsParameters,          // pointer to parameter descriptions
    SetFloat_Callback,          // Set float
    SetInt_Callback,            // Set int
    SetBool_Callback,           // Set bool
    SetData_Callback,           // Set data
    GetFloat_Callback,          // Get float
    GetInt_Callback,            // Get int
    GetBool_Callback,           // Get bool
    GetData_Callback,           // Get data
    ShouldIProcess_Callback,    // Check states before processing
    0,                          // User data
    SystemRegister_Callback,    // System register
    SystemDeregister_Callback,  // System deregister
    SystemMix_Callback          // Mixer thread exucute / after execute
};

extern "C"
{
    F_EXPORT FMOD_DSP_DESCRIPTION* F_CALL FMODGetDSPDescription()
    {
        FMOD_DSP_INIT_PARAMDESC_FLOAT(p_gain, "Gain", "dB", "Gain", -100, 10.0f, 0.0f, false);
        FMOD_DSP_INIT_PARAMDESC_FLOAT(p_pan, "Pan", "", "Panning", -1.0f, 1.0f, 0.0f);
        FMOD_DSP_INIT_PARAMDESC_FLOAT(p_cutoffFrequency, "Filter Cutoff", "Hz", "Filter Cutoff Frequency", 20.0f, 20000.0f, 20000.0f);
        return &PluginCallbacks;
    }
}

// ==================== //
//     PLUGIN CLASS     //
// ==================== //

class Plugin
{
public:
    Plugin() : m_delayLeft(), m_delayRight()
    {


    }


    void Read(float* inbuffer, float* outbuffer, unsigned int length, int channels);

    void SetGain(float value) { m_gain = value; }
    void SetPan(float value) { m_pan = value; }
    void SetCutoffFrequency(float value) 
    {
        float coeffs[5] = {};
        CalculateBiquadCoefficients(coeffs, Lowpass, value/20000.0f, 0.3f, 0);
    }


    float GetGain() { return m_gain; }
    float GetPan() { return m_pan; }
    float GetCutoffFrequency() { return 0; }

private:
    float m_gain = 1;
    float m_pan = 0;

    Biquad m_lowpassLeft;
    Biquad m_lowpassRight;

    Delay m_delayLeft;
    Delay m_delayRight;


    Reverb<8,3> reverb;

    BasicSOFA::BasicSOFA sofa;
    


};



void Plugin::Read(float* inbuffer, float* outbuffer, unsigned int length, int channels)
{
    unsigned int numSamples = length * channels;
    float* in = inbuffer;
    float* out = outbuffer;

    float totalGain = DbToLin(m_gain);
    float normPan = (m_pan * 0.5f + 0.5f) * pi * 0.5f;
    float gainLeft = cosf(normPan)*0.5f+0.5f;
    float gainRight = cosf(normPan - pi*0.5f) * 0.5f + 0.5f;


    m_lowpassLeft.CalculateCoefficients(Lowpass, fmaxf(gainLeft * 0.6f + 0.4f - 0.01f, 0.01f),0.3f,0);
    m_lowpassRight.CalculateCoefficients(Lowpass, fmaxf(gainRight * 0.6f + 0.4f - 0.01f, 0.01f), 0.3f, 0);

    m_delayLeft.SetDelayInMs(gainLeft*20);
    m_delayRight.SetDelayInMs(gainRight * 20);

    /*
    for (int i = 0; i < length; i++) 
    {
        for (int c = 0; c < channels; c++) {
            float input = *inbuffer++;

            switch (c) {
            case 0:
                input *= gainLeft;
                input = m_lowpassLeft.ProcessSample(input);
                input = m_delayLeft.ProcessSample(input);
                break;
            case 1:
                input *= gainRight;
                input = m_lowpassRight.ProcessSample(input);
                input = m_delayRight.ProcessSample(input);
                break;
            default:
                break;
            }

            *outbuffer++ = input * totalGain;
        }


    }
    */

    float channelBuffer[8] = {0,0,0,0,0,0,0,0};

    for (int i = 0; i < length; i+=1)
    {
        float inputLeft = *inbuffer++;
        float inputRight = *inbuffer++;

        inputLeft *= gainLeft;
        inputRight *= gainRight;

        inputLeft = m_lowpassLeft.ProcessSample(inputLeft);
        inputRight = m_lowpassRight.ProcessSample(inputRight);

        inputLeft = m_delayLeft.ProcessSample(inputLeft);
        inputRight = m_delayRight.ProcessSample(inputRight);
        
        for (int j = 0; j < 8; j++) {
            if (j < 4) channelBuffer[j] = inputLeft;
            else channelBuffer[j] = inputRight;
        }

        reverb.ProcessSample(channelBuffer);

        inputLeft = (channelBuffer[0] + channelBuffer[1] + channelBuffer[2] + channelBuffer[3]) * 0.33f;
        inputRight = (channelBuffer[4] + channelBuffer[5] + channelBuffer[6] + channelBuffer[7]) * 0.33f;
        
        *outbuffer++ = inputLeft;
        *outbuffer++ = inputRight;




    }

    
}


// ======================= //
// CALLBACK IMPLEMENTATION //
// ======================= //

FMOD_RESULT Create_Callback(FMOD_DSP_STATE* dsp_state)
{
    Plugin* state = (Plugin*)FMOD_DSP_ALLOC(dsp_state, sizeof(Plugin));
    dsp_state->plugindata = state;
    if (!dsp_state->plugindata)
    {
        return FMOD_ERR_MEMORY;
    }

    return FMOD_OK;
}

FMOD_RESULT Release_Callback(FMOD_DSP_STATE* dsp_state)
{
    Plugin* state = (Plugin*)dsp_state->plugindata;
    FMOD_DSP_FREE(dsp_state, state);

    return FMOD_OK;
}

FMOD_RESULT Reset_Callback(FMOD_DSP_STATE* dsp_state)
{
    return FMOD_OK;
}

FMOD_RESULT Read_Callback(FMOD_DSP_STATE* dsp_state, float* inbuffer, float* outbuffer, unsigned int length, int inchannels, int* outchannels)
{
    return FMOD_OK;
}

FMOD_RESULT Process_Callback(FMOD_DSP_STATE* dsp_state, unsigned int length, const FMOD_DSP_BUFFER_ARRAY* inbufferarray, FMOD_DSP_BUFFER_ARRAY* outbufferarray, FMOD_BOOL inputsidle, FMOD_DSP_PROCESS_OPERATION op)
{
    switch (op) {
    case FMOD_DSP_PROCESS_QUERY:
        if (outbufferarray && inbufferarray)
        {
            outbufferarray[0].bufferchannelmask[0] = inbufferarray[0].bufferchannelmask[0];
            outbufferarray[0].buffernumchannels[0] = inbufferarray[0].buffernumchannels[0];
            outbufferarray[0].speakermode = inbufferarray[0].speakermode;
        }

        if (inputsidle)
        {
            return FMOD_ERR_DSP_DONTPROCESS;
        }
        break;

    case FMOD_DSP_PROCESS_PERFORM:

        if (inputsidle)
        {
            return FMOD_ERR_DSP_DONTPROCESS;
        }

        Plugin* state = (Plugin*)dsp_state->plugindata;
        state->Read(inbufferarray[0].buffers[0], outbufferarray[0].buffers[0], length, outbufferarray[0].buffernumchannels[0]);

        break;
    }

    return FMOD_OK;
}

FMOD_RESULT SetPosition_Callback(FMOD_DSP_STATE* dsp_state, unsigned int pos)
{
    return FMOD_OK;
}

FMOD_RESULT ShouldIProcess_Callback(FMOD_DSP_STATE* dsp_state, FMOD_BOOL inputsidle, unsigned int length, FMOD_CHANNELMASK inmask, int inchannels, FMOD_SPEAKERMODE speakermode)
{
    if (inputsidle)
    {
        return FMOD_ERR_DSP_DONTPROCESS;
    }

    return FMOD_OK;
}

FMOD_RESULT SetFloat_Callback(FMOD_DSP_STATE* dsp_state, int index, float value)
{
    Plugin* state = (Plugin*)dsp_state->plugindata;
    switch (index) {
    case 0:
        state->SetGain(value);
        break;
    case 1:
        state->SetPan(value);
        break;
    case 2:
        state->SetCutoffFrequency(value);
        break;
    }

    return FMOD_OK;
}

FMOD_RESULT SetInt_Callback(FMOD_DSP_STATE* dsp_state, int index, int value)
{
    return FMOD_OK;
}

FMOD_RESULT SetBool_Callback(FMOD_DSP_STATE* dsp_state, int index, FMOD_BOOL value)
{
    return FMOD_OK;
}

FMOD_RESULT SetData_Callback(FMOD_DSP_STATE* dsp_state, int index, void* data, unsigned int length)
{
    return FMOD_OK;
}

FMOD_RESULT GetFloat_Callback(FMOD_DSP_STATE* dsp_state, int index, float* value, char* valuestr)
{
    Plugin* state = (Plugin*)dsp_state->plugindata;
    switch (index) {
    case 0:
        *value = state->GetGain();
        break;
    case 1:
        *value = state->GetPan();
        break;
    case 2:
        *value = state->GetCutoffFrequency();
        break;
    default:
        break;
    }

    return FMOD_OK;
}

FMOD_RESULT GetInt_Callback(FMOD_DSP_STATE* dsp_state, int index, int* value, char* valuestr)
{
    return FMOD_OK;
}

FMOD_RESULT GetBool_Callback(FMOD_DSP_STATE* dsp_state, int index, FMOD_BOOL* value, char* valuestr)
{
    return FMOD_OK;
}

FMOD_RESULT GetData_Callback(FMOD_DSP_STATE* dsp_state, int index, void** data, unsigned int* length, char* valuestr)
{
    return FMOD_OK;
}

FMOD_RESULT SystemRegister_Callback(FMOD_DSP_STATE* dsp_state)
{
    return FMOD_OK;
}

FMOD_RESULT SystemDeregister_Callback(FMOD_DSP_STATE* dsp_state)
{
    return FMOD_OK;
}

FMOD_RESULT SystemMix_Callback(FMOD_DSP_STATE* dsp_state, int stage)
{
    return FMOD_OK;
}