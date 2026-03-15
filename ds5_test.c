/**
 * DualSense Haptic & Audio Diagnostic Tool (Proton/SDL2 Version)
 *
 * Synchronized with native ALSA diagnostic logic:
 * - 440Hz for Speaker Channels (0, 1)
 * - 80Hz for Haptic Channels (2, 3)
 * - Sequential channel testing to verify routing accuracy.
 */

#include <SDL2/SDL.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

/* Constants aligned with native hardware tests */
#define SAMPLE_RATE 48000
#define CHANNELS    4
#define AUDIO_FREQ  440.0f   /* Standard A4 Pitch */
#define HAPTIC_FREQ 80.0f    /* Resonant frequency for DualSense VCM motors */
#define AMPLITUDE   22936    /* Approx 0.7 * 32767 (prevents clipping) */

/* Global state for the audio callback */
int g_active_channel = -1;

/**
 * SDL Audio Callback
 * Generates sine waves dynamically based on the active channel index.
 */
void audio_callback(void* userdata, Uint8* stream, int len) {
    static float phase = 0;
    int16_t* buffer = (int16_t*)stream;
    int total_samples = len / 2; /* len is in bytes, convert to 16-bit samples */

    /* Reset buffer to silence every cycle */
    memset(stream, 0, len);

    /* If no channel is active, remain silent */
    if (g_active_channel < 0) return;

    /* Determine frequency based on channel type (Audio vs Haptic) */
    float freq = (g_active_channel < 2) ? AUDIO_FREQ : HAPTIC_FREQ;
    float phase_step = (2.0f * (float)M_PI * freq) / (float)SAMPLE_RATE;

    for (int i = 0; i < total_samples; i += CHANNELS) {
        float val = sinf(phase);
        int16_t sample = (int16_t)(val * AMPLITUDE);

        /* Write sample only to the targeted channel index */
        if (g_active_channel < CHANNELS) {
            buffer[i + g_active_channel] = sample;
        }

        phase += phase_step;
        if (phase >= 2.0f * (float)M_PI) phase -= 2.0f * (float)M_PI;
    }
}

int main(int argc, char* argv[]) {
    printf("[INIT] DualSense Haptic Diagnostic Tool\n");

    /* Force WASAPI driver in Proton to bypass unnecessary software mixing */
    SDL_SetHint(SDL_HINT_AUDIODRIVER, "wasapi");
    /* Ensure basic resampling to maintain signal integrity for VCM motors */
    SDL_SetHint(SDL_HINT_AUDIO_RESAMPLING_MODE, "basic");

    if (SDL_Init(SDL_INIT_GAMECONTROLLER | SDL_INIT_AUDIO) < 0) {
        printf("[ERROR] SDL Init failed: %s\n", SDL_GetError());
        return 1;
    }

    /* 1. Controller Detection (Keeps the device awake) */
    SDL_GameController* controller = NULL;
    for (int i = 0; i < SDL_NumJoysticks(); ++i) {
        if (SDL_IsGameController(i)) {
            controller = SDL_GameControllerOpen(i);
            if (controller) {
                printf("[INFO] Controller: %s\n", SDL_GameControllerName(controller));
                break;
            }
        }
    }

    /* 2. Audio Device Enumeration */
    int nAudioDevices = SDL_GetNumAudioDevices(0);
    const char* ds5_audio_name = NULL;
    printf("[DEBUG] Found %d audio output devices\n", nAudioDevices);

    for (int i = 0; i < nAudioDevices; i++) {
        const char* name = SDL_GetAudioDeviceName(i, 0);
        printf("  [%d] %s\n", i, name);
        if (strstr(name, "DualSense") || strstr(name, "Wireless Controller")) {
            ds5_audio_name = name;
            break;
        }
    }

    /* 3. Open Audio Device with Quadraphonic Request */
    SDL_AudioSpec want, have;
    SDL_zero(want);
    want.freq = SAMPLE_RATE;
    want.format = AUDIO_S16SYS;
    want.channels = CHANNELS;
    want.samples = 1024; /* Small buffer for low latency */
    want.callback = audio_callback;

    SDL_AudioDeviceID dev = SDL_OpenAudioDevice(ds5_audio_name, 0, &want, &have, 0);
    if (dev == 0) {
        printf("[FATAL] Could not open audio device: %s\n", SDL_GetError());
        return 1;
    }

    printf("[INFO] Opened %d channels at %dHz\n", have.channels, have.freq);
    if (have.channels < 4) {
        printf("[WARN] OS provided less than 4 channels. Haptics will fail.\n");
    }

    const char* channel_labels[] = {
        "Channel 0: Front Left (Speaker)",
        "Channel 1: Front Right (Speaker)",
        "Channel 2: Rear Left (HAPTIC MOTOR)",
        "Channel 3: Rear Right (HAPTIC MOTOR)"
    };

    /* Start the audio stream */
    SDL_PauseAudioDevice(dev, 0);

    /* Sequential Test Loop */
    for (int i = 0; i < 4; i++) {
        printf("\n--- Ready to test %s ---\n", channel_labels[i]);
        printf("Press ENTER to start 2.0s signal...");
        getchar();

        g_active_channel = i;
        printf("[PLAYING] Frequency: %.1f Hz\n", (i < 2 ? AUDIO_FREQ : HAPTIC_FREQ));
        SDL_Delay(2000);

        g_active_channel = -1;
        printf("[STOPPED]\n");
    }

    /* Cleanup */
    printf("\n[CLEANUP] Closing devices...\n");
    SDL_CloseAudioDevice(dev);
    if (controller) SDL_GameControllerClose(controller);
    SDL_Quit();

    printf("[EXIT] Diagnostic complete. Press ENTER to close.\n");
    getchar();
    return 0;
}
