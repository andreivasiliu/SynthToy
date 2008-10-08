#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>

#include "modwinmm.h"


typedef struct _ModWinmmState
{
    HANDLE audio_thread;
    HANDLE event;
    
    WAVEFORMATEX wave_format;
    HWAVEOUT wave_out;
    HMIDIIN midi_in;
    
    ProcessCallback process_func;
    EventCallback event_func;
    void *instance_data;
} ModWinmmState;


void CALLBACK midi_func( HMIDIIN hMidiIn,
                UINT wMsg,
                DWORD_PTR dwInstance,
                DWORD_PTR dwParam1,
                DWORD_PTR dwParam2 )
{
    ModWinmmState *state = (ModWinmmState*) dwInstance;
    
    (void) hMidiIn;   /* Unused parameter. */
    (void) dwParam2;  /* Unused parameter. */
    
    if ( wMsg == MIM_DATA )
    {
        char message[3];
        
        message[0] = (dwParam1 >>  0) & 0xFF;
        message[1] = (dwParam1 >>  8) & 0xFF;
        message[2] = (dwParam1 >> 16) & 0xFF;
        
        if ( state->event_func )
            state->event_func(0, 1, message, 3, state->instance_data);
    }
}


void write_audio(ModWinmmState *state, void *memory_block)
{
    WAVEHDR *wave_header;
    float *new_block = NULL;
    int i, error;
    
    if ( memory_block )
        new_block = memory_block;
    else
        new_block = calloc(512, sizeof(float));
    
    state->process_func(NULL, new_block, 512, 44100, state->instance_data);
    
    for ( i = 0; i < 512; i++ )
        new_block[i] = new_block[i] * 0.2;
    
    wave_header = calloc(1, sizeof(WAVEHDR));
    
    wave_header->lpData = (void*) new_block;
    wave_header->dwBufferLength = 512*sizeof(float);
    wave_header->dwFlags = 0;
    wave_header->dwLoops = 0;
    
    error = waveOutPrepareHeader(state->wave_out, wave_header, sizeof(WAVEHDR));
    
    error = waveOutWrite(state->wave_out, wave_header, sizeof(WAVEHDR));
    
    if ( error != MMSYSERR_NOERROR )
    {
        char buf[256];
        waveOutGetErrorText(error, buf, 256);
        
        printf("Error2 text: %s.\n", buf);
    }
}


void CALLBACK wave_func(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance,
                        DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
    ModWinmmState *state = (ModWinmmState*) dwInstance;
    
    (void) dwParam2;  /* Unused parameter */
    
    if ( uMsg == WOM_DONE )
    {
        WAVEHDR *wave_header;
        char *block;
        
        if ( uMsg == WOM_DONE )
        {
            wave_header = (WAVEHDR*) dwParam1;
            
            waveOutUnprepareHeader(hwo, wave_header, sizeof(WAVEHDR));
            block = wave_header->lpData;
            free(wave_header);
            
            write_audio(state, block);
        }
    }
}


#ifndef WAVE_FORMAT_IEEE_FLOAT
# define WAVE_FORMAT_IEEE_FLOAT  0x0003
#endif

void *modwinmm_init(ProcessCallback process_func, EventCallback event_func,
                    void *arg, char **errmsg)
{
    ModWinmmState *state;
    LPWAVEFORMATEX wf;
    
    state = calloc(1, sizeof(ModWinmmState));
    wf = &state->wave_format;
    
    wf->wFormatTag = WAVE_FORMAT_IEEE_FLOAT;
    wf->nChannels = 1;
    wf->nSamplesPerSec = 44100L;
    wf->wBitsPerSample = sizeof(float)*8;
    
    wf->nBlockAlign = wf->nChannels * (wf->wBitsPerSample / 8);
    wf->nAvgBytesPerSec = wf->nBlockAlign * wf->nSamplesPerSec;
    wf->cbSize = 0;
    
    if ( waveOutOpen(NULL, WAVE_MAPPER, wf,
                     0, 0, WAVE_FORMAT_QUERY) != MMSYSERR_NOERROR )
    {
        *errmsg = "Format is not supported.";
        free(state);
        return NULL;
    }
    
    state->process_func = process_func;
    state->event_func = event_func;
    state->instance_data = arg;
    
    return state;
}
    
void modwinmm_fini(void *state)
{
    ModWinmmState *mw_state = (ModWinmmState*) state;
    
    free(mw_state);
}
    

void modwinmm_activate(void *state)
{
    ModWinmmState *mw_state = (ModWinmmState*) state;
    
    
    /* MIDI In */
    if ( midiInGetNumDevs() > 0 )
    {
        midiInOpen(&mw_state->midi_in, 0, (DWORD_PTR) midi_func,
                   (DWORD_PTR) mw_state, CALLBACK_FUNCTION);
        midiInStart(mw_state->midi_in);
    }
    else
    {
        puts("No MIDI device found, therefore no MIDI support for you.");
        fflush(stdout);
    }
    
    /* Audio Out */
    waveOutOpen(&mw_state->wave_out, WAVE_MAPPER, &mw_state->wave_format,
                (DWORD_PTR) wave_func, (DWORD_PTR) mw_state, CALLBACK_FUNCTION);
    
    /* Right here is where all your dreams of "low-latency" go down the drain. */
    write_audio(mw_state, NULL);
    write_audio(mw_state, NULL);
    write_audio(mw_state, NULL);
    write_audio(mw_state, NULL);
    write_audio(mw_state, NULL);
}

void modwinmm_deactivate(void *state)
{
    ModWinmmState *mw_state = (ModWinmmState*) state;
    
    mw_state = 0;
}
