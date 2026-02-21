/**
 * DualSense Haptic & Audio Diagnostic Tool (V6)
 *
 * Features:
 * - English comments for technical clarity.
 * - Adaptive Frequency: 440Hz for Audio, 80Hz for Haptics.
 * - High Verbosity: Detailed ALSA state and buffer tracking.
 *
 * Build: gcc -o ds_diag_v6 ds_diag_v6.c -lasound -lm
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <alsa/asoundlib.h>
#include <unistd.h>

#define SAMPLE_RATE 48000
#define CHANNELS    4
#define AUDIO_FREQ  440.0   /* Standard A4 for Speakers */
#define HAPTIC_FREQ 80.0    /* Deep Rumble for VCM Motors */
#define TEST_DURATION 1.5

/* Search for DualSense card index in ALSA system */
int find_dualsense_card() {
    int card = -1;
    snd_ctl_card_info_t *info;
    snd_ctl_card_info_alloca(&info);

    printf("[DEBUG] Enumerating ALSA sound cards...\n");
    while (snd_card_next(&card) == 0 && card >= 0) {
        char name[32];
        sprintf(name, "hw:%d", card);
        snd_ctl_t *handle;
        if (snd_ctl_open(&handle, name, 0) < 0) continue;

        snd_ctl_card_info(handle, info);
        const char* card_name = snd_ctl_card_info_get_name(info);
        printf("[DEBUG] Found Card %d: %s\n", card, card_name);

        if (strstr(card_name, "DualSense")) {
            snd_ctl_close(handle);
            return card;
        }
        snd_ctl_close(handle);
    }
    return -1;
}

/**
 * Play sine wave on a specific channel with adaptive frequency
 * Channel 0,1 -> 440Hz
 * Channel 2,3 -> 80Hz
 */
void test_channel(snd_pcm_t *handle, int channel_index) {
    const char* labels[] = {"Front Left (Audio)", "Front Right (Audio)",
        "Rear Left (Haptic)", "Rear Right (Haptic)"};

        /* Logic: Use 440Hz for audio channels, 80Hz for haptic channels */
        double freq = (channel_index < 2) ? AUDIO_FREQ : HAPTIC_FREQ;

        printf("\n[TESTING] Channel %d [%s]\n", channel_index, labels[channel_index]);
        printf("[CONFIG] Frequency set to %.1f Hz\n", freq);

        /* Ensure PCM device is prepared */
        snd_pcm_prepare(handle);

        snd_pcm_status_t *status;
        snd_pcm_status_alloca(&status);
        snd_pcm_status(handle, status);
        printf("[STATE] PCM State: %s\n", snd_pcm_state_name(snd_pcm_status_get_state(status)));

        long frames_to_play = SAMPLE_RATE * TEST_DURATION;
        short *buffer = (short *)calloc(frames_to_play * CHANNELS, sizeof(short));

        /* Sine wave generation with 0.7 amplitude to prevent clipping */
        for (long i = 0; i < frames_to_play; i++) {
            double t = (double)i / SAMPLE_RATE;
            short sample = (short)(0.7 * 32767.0 * sin(2.0 * M_PI * freq * t));
            buffer[i * CHANNELS + channel_index] = sample;
        }

        printf("[WRITE] Pushing %ld frames to ring buffer...\n", frames_to_play);
        snd_pcm_sframes_t frames_written = snd_pcm_writei(handle, buffer, frames_to_play);

        if (frames_written < 0) {
            fprintf(stderr, "[ERROR] Write failed: %s\n", snd_strerror(frames_written));
            snd_pcm_recover(handle, frames_written, 0);
        } else {
            printf("[SUCCESS] Buffer consumed by hardware DMA.\n");
        }

        /* Wait for playback to finish */
        printf("[WAIT] Draining stream...\n");
        snd_pcm_drain(handle);

        free(buffer);
}

int main() {
    int card_idx = find_dualsense_card();
    if (card_idx < 0) {
        fprintf(stderr, "[FATAL] Cannot find DualSense hardware via ALSA API.\n");
        return 1;
    }

    char pcm_name[64];
    sprintf(pcm_name, "plughw:%d,0", card_idx);
    printf("[START] Initializing PCM device: %s\n", pcm_name);

    snd_pcm_t *handle;
    if (snd_pcm_open(&handle, pcm_name, SND_PCM_STREAM_PLAYBACK, 0) < 0) {
        fprintf(stderr, "[FATAL] PCM device busy or unavailable. Check PipeWire/PulseAudio usage.\n");
        return 1;
    }

    /* Set Hardware Parameters */
    snd_pcm_hw_params_t *params;
    snd_pcm_hw_params_alloca(&params);
    snd_pcm_hw_params_any(handle, params);

    snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S16_LE);
    snd_pcm_hw_params_set_channels(handle, params, CHANNELS);

    unsigned int rate = SAMPLE_RATE;
    snd_pcm_hw_params_set_rate_near(handle, params, &rate, 0);
    snd_pcm_hw_params(handle, params);

    snd_pcm_uframes_t buffer_sz;
    snd_pcm_hw_params_get_buffer_size(params, &buffer_sz);
    printf("[INFO] ALSA Hardware Buffer: %lu frames\n", buffer_sz);

    /* Run tests for all 4 channels */
    for (int i = 0; i < 4; i++) {
        test_channel(handle, i);
        usleep(150000); /* 150ms delay for physical damping */
    }

    printf("\n[FINISH] Diagnostic complete. Motors and Speakers should have responded.\n");
    snd_pcm_close(handle);
    return 0;
}
