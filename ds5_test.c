#include <SDL2/SDL.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#define SAMPLE_RATE 48000
#define AMPLITUDE 28000
#define FREQUENCY 150.0f

void audio_callback(void* userdata, Uint8* stream, int len) {
    static float phase = 0;
    int16_t* buffer = (int16_t*)stream;
    int samples = len / 2;
    for (int i = 0; i < samples; i += 4) {
        float val = sinf(phase);
        buffer[i] = 0;     // Ch 0: Front L (Speaker)
        buffer[i+1] = 0;   // Ch 1: Front R (Speaker)
        buffer[i+2] = (int16_t)(val * AMPLITUDE); // Ch 2: Left Haptic Motor
        buffer[i+3] = (int16_t)(val * AMPLITUDE); // Ch 3: Right Haptic Motor
        phase += 2.0f * M_PI * FREQUENCY / SAMPLE_RATE;
        if (phase >= 2.0f * M_PI) phase -= 2.0f * M_PI;
    }
}

int main(int argc, char* argv[]) {
    printf("[DEBUG] Initializing SDL...\n");
    if (SDL_Init(SDL_INIT_GAMECONTROLLER | SDL_INIT_AUDIO) < 0) {
        printf("[ERROR] SDL Init Failed: %s\n", SDL_GetError());
        return 1;
    }

    // 1. 手柄检测
    SDL_GameController* controller = NULL;
    int nJoysticks = SDL_NumJoysticks();
    printf("[DEBUG] Found %d joysticks\n", nJoysticks);

    for (int i = 0; i < nJoysticks; ++i) {
        if (SDL_IsGameController(i)) {
            controller = SDL_GameControllerOpen(i);
            if (controller) {
                printf("[INFO] Controller Connected: %s\n", SDL_GameControllerName(controller));
                break;
            }
        }
    }

    if (!controller) {
        printf("[ERROR] No PS5/DualSense controller found.\n");
        return 1;
    }

    printf("\n--- Test 1: Separate Rumble ---\n");
    printf("Press ENTER to test LEFT motor (Low Frequency)..."); getchar();
    SDL_GameControllerRumble(controller, 0xFFFF, 0x0000, 1500);
    SDL_Delay(1500);

    printf("Press ENTER to test RIGHT motor (High Frequency)..."); getchar();
    SDL_GameControllerRumble(controller, 0x0000, 0xFFFF, 1500);
    SDL_Delay(1500);


    printf("\n--- Test 2: Audio Haptics (Channel 2/3) ---\n");

    int nAudioDevices = SDL_GetNumAudioDevices(0);
    printf("[DEBUG] Found %d audio output devices:\n", nAudioDevices);
    const char* ds5_audio_name = NULL;

    for (int i = 0; i < nAudioDevices; i++) {
        const char* name = SDL_GetAudioDeviceName(i, 0);
        printf("  [%d] %s\n", i, name);
        if (strstr(name, "Wireless") || strstr(name, "DualSense") || strstr(name, "Controller")) {
            ds5_audio_name = name;
        }
    }

    if (!ds5_audio_name) {
        printf("[WARN] Could not find a device name containing 'Wireless' or 'DualSense'.\n");
        printf("[WARN] Falling back to default device (NULL).\n");
    } else {
        printf("[INFO] Target Audio Device: %s\n", ds5_audio_name);
    }

    SDL_AudioSpec want, have;
    SDL_zero(want);
    want.freq = SAMPLE_RATE;
    want.format = AUDIO_S16SYS;
    want.channels = 4;
    want.samples = 4096;
    want.callback = audio_callback;

    SDL_AudioDeviceID dev = SDL_OpenAudioDevice(ds5_audio_name, 0, &want, &have, 0);

    if (dev == 0) {
        printf("[ERROR] Failed to open audio device: %s\n", SDL_GetError());
    } else {
        printf("[DEBUG] Opened audio with %d channels. If you hear sound from speakers, \n", have.channels);
        printf("[DEBUG] your system is NOT routing this device to the DualSense motors.\n");
        printf("Press ENTER to start 150Hz vibration on Ch 2/3..."); getchar();

        SDL_PauseAudioDevice(dev, 0); // Start playing
        SDL_Delay(3000);
        SDL_CloseAudioDevice(dev);
    }

    printf("\nTest Complete.\n");
    SDL_GameControllerClose(controller);
    SDL_Quit();
    return 0;
}
