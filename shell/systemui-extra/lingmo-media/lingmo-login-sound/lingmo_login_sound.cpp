/*
 *  aplay.c - plays and records
 *
 *      CREATIVE LABS CHANNEL-files
 *      Microsoft WAVE-files
 *      SPARC AUDIO .AU-files
 *      Raw Data
 *
 *  Copyright (c) by Jaroslav Kysela <perex@perex.cz>
 *  Based on vplay program by Michael Beck
 *
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */
#include "lingmo_login_sound.h"

FILE *pidf = NULL;

LingmoUILoginSound& LingmoUILoginSound::getInstance()
{
    static LingmoUILoginSound instance;
    return instance;
}

void LingmoUILoginSound::device_list()
{
    snd_ctl_t *handle;
    int card, err, dev, idx;
    snd_ctl_card_info_t *info;
    snd_pcm_info_t *pcminfo;
    snd_ctl_card_info_alloca(&info);
    snd_pcm_info_alloca(&pcminfo);

    card = -1;
    if (snd_card_next(&card) < 0 || card < 0) {
        printf(("no soundcards found..."));
        return;
    }
    printf(("**** List of %s Hardware Devices ****\n"),
           snd_pcm_stream_name(stream));
    while (card >= 0) {
        char name[32];
        sprintf(name, "hw:%d", card);
        if ((err = snd_ctl_open(&handle, name, 0)) < 0) {
            printf("control open (%i): %s", card, snd_strerror(err));
            goto next_card;
        }
        if ((err = snd_ctl_card_info(handle, info)) < 0) {
            printf("control hardware info (%i): %s", card, snd_strerror(err));
            snd_ctl_close(handle);
            goto next_card;
        }
        dev = -1;
        while (1) {
            unsigned int count;
            if (snd_ctl_pcm_next_device(handle, &dev)<0)
                printf("snd_ctl_pcm_next_device");
            if (dev < 0)
                break;
            snd_pcm_info_set_device(pcminfo, dev);
            snd_pcm_info_set_subdevice(pcminfo, 0);
            snd_pcm_info_set_stream(pcminfo, stream);
            if ((err = snd_ctl_pcm_info(handle, pcminfo)) < 0) {
                if (err != -ENOENT)
                    printf("control digital audio info (%i): %s", card, snd_strerror(err));
                continue;
            }
            printf(("card %i: %s [%s], device %i: %s [%s]\n"),
                   card, snd_ctl_card_info_get_id(info), snd_ctl_card_info_get_name(info),
                   dev,
                   snd_pcm_info_get_id(pcminfo),
                   snd_pcm_info_get_name(pcminfo));
            count = snd_pcm_info_get_subdevices_count(pcminfo);
            printf(("  Subdevices: %i/%i\n"),
                   snd_pcm_info_get_subdevices_avail(pcminfo), count);
            for (idx = 0; idx < (int)count; idx++) {
                snd_pcm_info_set_subdevice(pcminfo, idx);
                if ((err = snd_ctl_pcm_info(handle, pcminfo)) < 0) {
                    printf("control digital audio playback info (%i): %s", card, snd_strerror(err));
                } else {
                    printf(("  Subdevice #%i: %s\n"),
                           idx, snd_pcm_info_get_subdevice_name(pcminfo));
                }
            }
        }
        snd_ctl_close(handle);
next_card:
        if (snd_card_next(&card) < 0) {
            printf("snd_card_next");
            break;
        }
    }
}

void LingmoUILoginSound::pcm_list(void)
{
    void **hints, **n;
    char *name, *descr, *descr1, *io;
    const char *filter;

    if (snd_device_name_hint(-1, "pcm", &hints) < 0)
        return;
    n = hints;
    filter = stream == SND_PCM_STREAM_CAPTURE ? "Input" : "Output";
    while (*n != NULL) {
        name = snd_device_name_get_hint(*n, "NAME");
        descr = snd_device_name_get_hint(*n, "DESC");
        io = snd_device_name_get_hint(*n, "IOID");
        if (io != NULL && strcmp(io, filter) != 0)
            goto __end;
        printf("%s\n", name);
        if ((descr1 = descr) != NULL) {
            printf("    ");
            while (*descr1) {
                if (*descr1 == '\n')
                    printf("\n    ");
                else
                    putchar(*descr1);
                descr1++;
            }
            putchar('\n');
        }
__end:
        if (name != NULL)
            free(name);
        if (descr != NULL)
            free(descr);
        if (io != NULL)
            free(io);
        n++;
    }
    snd_device_name_free_hint(hints);
}


/*
 *	Subroutine to clean up before exit.
 */
void LingmoUILoginSound::prg_exit(int code)
{
    done_stdin();
    if (handle)
        snd_pcm_close(handle);
    if (pidfile_written)
        remove (pidfile_name);
    exit(code);
}

void LingmoUILoginSound::signal_handler(int sig)
{
    if (in_aborting)
        return;

    in_aborting = 1;
    if (verbose==2)
        putchar('\n');
    if (!quiet_mode)
    {
        syslog (LOG_INFO,"sig: %d quiet_mode: %d.", sig, quiet_mode);
        fprintf(stderr, ("Aborted by signal %s...\n"), strsignal(sig));
    }

    if (handle)
        snd_pcm_abort(handle);
    if (sig == SIGABRT) {
        /* do not call snd_pcm_close() and abort immediately */
        handle = NULL;
        prg_exit(EXIT_FAILURE);
    }
    signal(sig, SIG_DFL);
}

/* call on SIGUSR1 signal. */
void LingmoUILoginSound::signal_handler_recycle ([[maybe_unused]] int sig)
{
    /* flag the capture loop to start a new output file */
    recycle_capture_file = 1;
}

int LingmoUILoginSound::aplayMain(char *filename, char *playdev)
{
//    int duration_or_sample = 0;
//    int option_index;
//    static const char short_options[] = "hnlLD:qt:c:f:r:d:s:MNF:A:R:T:B:vV:IPCi"

#ifdef CONFIG_SUPPORT_CHMAP
    "m:"
#endif
    ;

    char *pcm_name = playdev;
    int err;
    int do_device_list = 0, do_pcm_list = 0;
    snd_pcm_info_t *info;

#ifdef ENABLE_NLS
    setlocale(LC_ALL, "");
    textdomain(PACKAGE);
#endif

    snd_pcm_info_alloca(&info);

    err = snd_output_stdio_attach(&log, stderr, 0);
    assert(err >= 0);

    file_type = FORMAT_DEFAULT;
    stream = SND_PCM_STREAM_PLAYBACK;

    chunk_size = -1;
    rhwparams.format = DEFAULT_FORMAT;
    rhwparams.rate = DEFAULT_SPEED;
    rhwparams.channels = 1;

    if (do_device_list) {
        if (do_pcm_list)
            pcm_list();
        device_list();
        goto __end;
    } else if (do_pcm_list) {
        pcm_list();
        goto __end;
    }

    err = snd_pcm_open(&handle, playdev, stream, open_mode);
    syslog (LOG_INFO, "start open %s device...", pcm_name);
    if (err < 0) {
        printf(("audio open error: %s"), snd_strerror(err));
        return 1;
    }

    if ((err = snd_pcm_info(handle, info)) < 0) {
        printf(("info error: %s"), snd_strerror(err));
        return 1;
    }

    if (nonblock) {
        err = snd_pcm_nonblock(handle, 1);
        if (err < 0) {
            printf(("nonblock setting error: %s"), snd_strerror(err));
            return 1;
        }
    }

    chunk_size = 1024;
    hwparams = rhwparams;

    audiobuf = (u_char *)malloc(1024);
    if (audiobuf == NULL) {
        printf(("not enough memory"));
        return 1;
    }

    if (mmap_flag) {
        writei_func = snd_pcm_mmap_writei;
        readi_func = snd_pcm_mmap_readi;
        writen_func = snd_pcm_mmap_writen;
        readn_func = snd_pcm_mmap_readn;
    } else {
        writei_func = snd_pcm_writei;
        readi_func = snd_pcm_readi;
        writen_func = snd_pcm_writen;
        readn_func = snd_pcm_readn;
    }

    if (pidfile_name) {
        errno = 0;
        pidf = fopen (pidfile_name, "w");
        if (pidf) {
            (void)fprintf (pidf, "%d\n", getpid());
            fclose(pidf);
            pidfile_written = 1;
        } else {
            printf(("Cannot create process ID file %s: %s"),
                   pidfile_name, strerror (errno));
            return 1;
        }
    }

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGABRT, signal_handler);
    signal(SIGUSR1, signal_handler_recycle);
    if (interleaved) {
        if (stream == SND_PCM_STREAM_PLAYBACK)
        {
            playback(filename);
        }

    }

    if (verbose==2)
        putchar('\n');
    snd_pcm_close(handle);
    handle = NULL;
    free(audiobuf);
__end:
    snd_output_close(log);
    snd_config_update_free_global();
    prg_exit(EXIT_SUCCESS);
    /* avoid warning */
    return EXIT_SUCCESS;
}

/*
 * Safe read (for pipes)
 */

ssize_t LingmoUILoginSound::safe_read(int fd, void *buf, size_t count)
{
    ssize_t result = 0, res;

    while (count > 0 && !in_aborting) {
        if ((res = read(fd, buf, count)) == 0)
            break;
        if (res < 0)
            return result > 0 ? result : res;
        count -= res;
        result += res;
        buf = (char *)buf + res;
    }
    return result;
}

/*
 * helper for test_wavefile
 */

size_t LingmoUILoginSound::test_wavefile_read(int fd, u_char *buffer, size_t *size, size_t reqsize, int line)
{
    if (*size >= reqsize)
        return *size;
    if ((size_t)safe_read(fd, buffer + *size, reqsize - *size) != reqsize - *size) {
        printf(("read error (called from line %i)"), line);
        prg_exit(EXIT_FAILURE);
    }
    return *size = reqsize;
}

#define check_wavefile_space(buffer, len, blimit) \
    if (len > blimit) { \
    blimit = len; \
    if ((buffer = (u_char *)realloc(buffer, blimit)) == NULL) { \
    printf(("not enough memory"));		  \
    prg_exit(EXIT_FAILURE);  \
    } \
    }

/*
 * test, if it's a .WAV file, > 0 if ok (and set the speed, stereo etc.)
 *                            == 0 if not
 * Value returned is bytes to be discarded.
 */
ssize_t LingmoUILoginSound::test_wavefile(int fd, u_char *_buffer, size_t size)
{
    WaveHeader *h = (WaveHeader *)_buffer;
    u_char *buffer = NULL;
    size_t blimit = 0;
    WaveFmtBody *f;
    WaveChunkHeader *c;
    u_int type, len;
    unsigned short format, channels;
    int big_endian, native_format;

    if (size < sizeof(WaveHeader))
        return -1;
    if (h->magic == WAV_RIFF)
        big_endian = 0;
    else if (h->magic == WAV_RIFX)
        big_endian = 1;
    else
        return -1;
    if (h->type != WAV_WAVE)
        return -1;

    if (size > sizeof(WaveHeader)) {
        check_wavefile_space(buffer, size - sizeof(WaveHeader), blimit);
        memcpy(buffer, (_buffer + (sizeof(WaveHeader))), size - sizeof(WaveHeader));
    }

    size -= sizeof(WaveHeader);
    while (1) {
        check_wavefile_space(buffer, sizeof(WaveChunkHeader), blimit);
        test_wavefile_read(fd, buffer, &size, sizeof(WaveChunkHeader), __LINE__);
        c = (WaveChunkHeader*)buffer;
        type = c->type;
        len = TO_CPU_INT(c->length, big_endian);
        len += len % 2;
        if (size > sizeof(WaveChunkHeader))
            memmove(buffer, buffer + sizeof(WaveChunkHeader), size - sizeof(WaveChunkHeader));
        size -= sizeof(WaveChunkHeader);
        if (type == WAV_FMT)
            break;
        check_wavefile_space(buffer, len, blimit);
        test_wavefile_read(fd, buffer, &size, len, __LINE__);
        if (size > len)
            memmove(buffer, buffer + len, size - len);
        size -= len;
    }

    if (len < sizeof(WaveFmtBody)) {
        printf(("unknown length of 'fmt ' chunk (read %u, should be %u at least)"),
               len, (u_int)sizeof(WaveFmtBody));
        prg_exit(EXIT_FAILURE);
    }
    check_wavefile_space(buffer, len, blimit);
    test_wavefile_read(fd, buffer, &size, len, __LINE__);
    f = (WaveFmtBody*) buffer;
    format = TO_CPU_SHORT(f->format, big_endian);
    if (format == WAV_FMT_EXTENSIBLE) {
        WaveFmtExtensibleBody *fe = (WaveFmtExtensibleBody*)buffer;
        if (len < sizeof(WaveFmtExtensibleBody)) {
            printf(("unknown length of extensible 'fmt ' chunk (read %u, should be %u at least)"),
                   len, (u_int)sizeof(WaveFmtExtensibleBody));
            prg_exit(EXIT_FAILURE);
        }
        if (memcmp(fe->guid_tag, WAV_GUID_TAG, 14) != 0) {
            printf(("wrong format tag in extensible 'fmt ' chunk"));
            prg_exit(EXIT_FAILURE);
        }
        format = TO_CPU_SHORT(fe->guid_format, big_endian);
    }
    if (format != WAV_FMT_PCM &&
            format != WAV_FMT_IEEE_FLOAT) {
        printf(("can't play WAVE-file format 0x%04x which is not PCM or FLOAT encoded"), format);
        prg_exit(EXIT_FAILURE);
    }
    channels = TO_CPU_SHORT(f->channels, big_endian);
    if (channels < 1) {
        printf(("can't play WAVE-files with %d tracks"), channels);
        prg_exit(EXIT_FAILURE);
    }
    hwparams.channels = channels;
    switch (TO_CPU_SHORT(f->bit_p_spl, big_endian)) {
    case 8:
        if (hwparams.format != DEFAULT_FORMAT &&
                hwparams.format != SND_PCM_FORMAT_U8)
            fprintf(stderr, ("Warning: format is changed to U8\n"));
        hwparams.format = SND_PCM_FORMAT_U8;
        break;
    case 16:
        if (big_endian)
            native_format = SND_PCM_FORMAT_S16_BE;
        else
            native_format = SND_PCM_FORMAT_S16_LE;
        if (hwparams.format != DEFAULT_FORMAT &&
                hwparams.format != native_format)
            fprintf(stderr, ("Warning: format is changed to %s\n"),
                    snd_pcm_format_name((snd_pcm_format_t)native_format));
        hwparams.format = (snd_pcm_format_t)native_format;
        break;
    case 24:
        switch (TO_CPU_SHORT(f->byte_p_spl, big_endian) / hwparams.channels) {
        case 3:
            if (big_endian)
                native_format = SND_PCM_FORMAT_S24_3BE;
            else
                native_format = SND_PCM_FORMAT_S24_3LE;
            if (hwparams.format != DEFAULT_FORMAT &&
                    hwparams.format != native_format)
                fprintf(stderr, ("Warning: format is changed to %s\n"),
                        snd_pcm_format_name((snd_pcm_format_t)native_format));
            hwparams.format = (snd_pcm_format_t)native_format;
            break;
        case 4:
            if (big_endian)
                native_format = SND_PCM_FORMAT_S24_BE;
            else
                native_format = SND_PCM_FORMAT_S24_LE;
            if (hwparams.format != DEFAULT_FORMAT &&
                    hwparams.format != native_format)
                fprintf(stderr, ("Warning: format is changed to %s\n"),
                        snd_pcm_format_name((snd_pcm_format_t)native_format));
            hwparams.format = (snd_pcm_format_t)native_format;
            break;
        default:
            printf((" can't play WAVE-files with sample %d bits in %d bytes wide (%d channels)"),
                   TO_CPU_SHORT(f->bit_p_spl, big_endian),
                   TO_CPU_SHORT(f->byte_p_spl, big_endian),
                   hwparams.channels);
            prg_exit(EXIT_FAILURE);
        }
        break;
    case 32:
        if (format == WAV_FMT_PCM) {
            if (big_endian)
                native_format = SND_PCM_FORMAT_S32_BE;
            else
                native_format = SND_PCM_FORMAT_S32_LE;
            hwparams.format = (snd_pcm_format_t)native_format;
        } else if (format == WAV_FMT_IEEE_FLOAT) {
            if (big_endian)
                native_format = SND_PCM_FORMAT_FLOAT_BE;
            else
                native_format = SND_PCM_FORMAT_FLOAT_LE;
            hwparams.format = (snd_pcm_format_t)native_format;
        }
        break;
    default:
        printf((" can't play WAVE-files with sample %d bits wide"),
               TO_CPU_SHORT(f->bit_p_spl, big_endian));
        prg_exit(EXIT_FAILURE);
    }
    hwparams.rate = TO_CPU_INT(f->sample_fq, big_endian);

    if (size > len)
        memmove(buffer, buffer + len, size - len);
    size -= len;

    while (1) {
        u_int type, len;

        check_wavefile_space(buffer, sizeof(WaveChunkHeader), blimit);
        test_wavefile_read(fd, buffer, &size, sizeof(WaveChunkHeader), __LINE__);
        c = (WaveChunkHeader*)buffer;
        type = c->type;
        len = TO_CPU_INT(c->length, big_endian);
        if (size > sizeof(WaveChunkHeader))
            memmove(buffer, buffer + sizeof(WaveChunkHeader), size - sizeof(WaveChunkHeader));
        size -= sizeof(WaveChunkHeader);
        if (type == WAV_DATA) {
            if (len < pbrec_count && len < 0x7ffffffe)
                pbrec_count = len;
            if (size > 0)
                memcpy(_buffer, buffer, size);
            free(buffer);
            return size;
        }
        len += len % 2;
        check_wavefile_space(buffer, len, blimit);
        test_wavefile_read(fd, buffer, &size, len, __LINE__);
        if (size > len)
            memmove(buffer, buffer + len, size - len);
        size -= len;
    }

    /* shouldn't be reached */
    return -1;
}

void LingmoUILoginSound::show_available_sample_formats(snd_pcm_hw_params_t* params)
{
    //    snd_pcm_format_t format;
    int format;

    fprintf(stderr, "Available formats:\n");
    for (format = 0; format <= SND_PCM_FORMAT_LAST; format++) {
        if (snd_pcm_hw_params_test_format(handle, params, (snd_pcm_format_t)format) == 0)
            fprintf(stderr, "- %s\n", snd_pcm_format_name((snd_pcm_format_t)format));
    }
}

#ifdef CONFIG_SUPPORT_CHMAP
int LingmoUILoginSound::setup_chmap(void)
{
    snd_pcm_chmap_t *chmap = channel_map;
    char mapped[hwparams.channels];
    snd_pcm_chmap_t *hw_chmap;
    unsigned int ch, i;
    int err;

    if (!chmap)
        return 0;

    if (chmap->channels != hwparams.channels) {
        printf(("Channel numbers don't match between hw_params and channel map"));
        return -1;
    }
    err = snd_pcm_set_chmap(handle, chmap);
    if (!err)
        return 0;

    hw_chmap = snd_pcm_get_chmap(handle);
    if (!hw_chmap) {
        fprintf(stderr, ("Warning: unable to get channel map\n"));
        return 0;
    }

    if (hw_chmap->channels == chmap->channels &&
            !memcmp(hw_chmap, chmap, 4 * (chmap->channels + 1))) {
        /* maps are identical, so no need to convert */
        free(hw_chmap);
        return 0;
    }

    hw_map = (unsigned int *)calloc(hwparams.channels, sizeof(int));
    if (!hw_map) {
        printf(("not enough memory"));
        free(hw_chmap);
        return -1;
    }

    memset(mapped, 0, sizeof(mapped));
    for (ch = 0; ch < hw_chmap->channels; ch++) {
        if (chmap->pos[ch] == hw_chmap->pos[ch]) {
            mapped[ch] = 1;
            hw_map[ch] = ch;
            continue;
        }
        for (i = 0; i < hw_chmap->channels; i++) {
            if (!mapped[i] && chmap->pos[ch] == hw_chmap->pos[i]) {
                mapped[i] = 1;
                hw_map[ch] = i;
                break;
            }
        }
        if (i >= hw_chmap->channels) {
            char buf[256];
            printf(("Channel %d doesn't match with hw_parmas"), ch);
            snd_pcm_chmap_print(hw_chmap, sizeof(buf), buf);
            fprintf(stderr, "hardware chmap = %s\n", buf);
            free(hw_chmap);
            return -1;
        }
    }
    free(hw_chmap);
    return 0;
}
#else
#define setup_chmap()	0
#endif

void LingmoUILoginSound::set_params(void)
{
    snd_pcm_hw_params_t *params;
    snd_pcm_sw_params_t *swparams;
    snd_pcm_uframes_t buffer_size;
    int err;
    size_t n;
    unsigned int rate;
    snd_pcm_uframes_t start_threshold, stop_threshold;
    snd_pcm_hw_params_alloca(&params);
    snd_pcm_sw_params_alloca(&swparams);
    err = snd_pcm_hw_params_any(handle, params);
    if (err < 0) {
        printf(("Broken configuration for this PCM: no configurations available"));
        prg_exit(EXIT_FAILURE);
    }
    if (dump_hw_params) {
        fprintf(stderr, ("HW Params of device \"%s\":\n"),
                snd_pcm_name(handle));
        fprintf(stderr, "--------------------\n");
        snd_pcm_hw_params_dump(params, log);
        fprintf(stderr, "--------------------\n");
    }
    if (mmap_flag) {
        snd_pcm_access_mask_t *mask = (snd_pcm_access_mask_t *)alloca(snd_pcm_access_mask_sizeof());
        snd_pcm_access_mask_none(mask);
        snd_pcm_access_mask_set(mask, SND_PCM_ACCESS_MMAP_INTERLEAVED);
        snd_pcm_access_mask_set(mask, SND_PCM_ACCESS_MMAP_NONINTERLEAVED);
        snd_pcm_access_mask_set(mask, SND_PCM_ACCESS_MMAP_COMPLEX);
        err = snd_pcm_hw_params_set_access_mask(handle, params, mask);
    } else if (interleaved)
        err = snd_pcm_hw_params_set_access(handle, params,
                                           SND_PCM_ACCESS_RW_INTERLEAVED);
    else
        err = snd_pcm_hw_params_set_access(handle, params,
                                           SND_PCM_ACCESS_RW_NONINTERLEAVED);
    if (err < 0) {
        printf(("Access type not available"));
        prg_exit(EXIT_FAILURE);
    }
    err = snd_pcm_hw_params_set_format(handle, params, hwparams.format);
    if (err < 0) {
        printf(("Sample format non available"));
        show_available_sample_formats(params);
        prg_exit(EXIT_FAILURE);
    }
    err = snd_pcm_hw_params_set_channels(handle, params, hwparams.channels);
    if (err < 0) {
        printf(("Channels count non available"));
        prg_exit(EXIT_FAILURE);
    }

#if 0
    err = snd_pcm_hw_params_set_periods_min(handle, params, 2);
    assert(err >= 0);
#endif
    rate = hwparams.rate;
    err = snd_pcm_hw_params_set_rate_near(handle, params, &hwparams.rate, 0);
    assert(err >= 0);
    if ((float)rate * 1.05 < hwparams.rate || (float)rate * 0.95 > hwparams.rate) {
        if (!quiet_mode) {
            char plugex[64];
            const char *pcmname = snd_pcm_name(handle);
            fprintf(stderr, ("Warning: rate is not accurate (requested = %iHz, got = %iHz)\n"), rate, hwparams.rate);
            if (! pcmname || strchr(snd_pcm_name(handle), ':'))
                *plugex = 0;
            else
                snprintf(plugex, sizeof(plugex), "(-Dplug:%s)",
                         snd_pcm_name(handle));
            fprintf(stderr, ("         please, try the plug plugin %s\n"),
                    plugex);
        }
    }
    rate = hwparams.rate;
    if (buffer_time == 0 && buffer_frames == 0) {
        err = snd_pcm_hw_params_get_buffer_time_max(params,
                                                    &buffer_time, 0);
        assert(err >= 0);
        if (buffer_time > 500000)
            buffer_time = 500000;
    }
    if (period_time == 0 && period_frames == 0) {
        if (buffer_time > 0)
            period_time = buffer_time / 4;
        else
            period_frames = buffer_frames / 4;
    }
    if (period_time > 0)
        err = snd_pcm_hw_params_set_period_time_near(handle, params,
                                                     &period_time, 0);
    else
        err = snd_pcm_hw_params_set_period_size_near(handle, params,
                                                     &period_frames, 0);
    assert(err >= 0);
    if (buffer_time > 0) {
        err = snd_pcm_hw_params_set_buffer_time_near(handle, params,
                                                     &buffer_time, 0);
    } else {
        err = snd_pcm_hw_params_set_buffer_size_near(handle, params,
                                                     &buffer_frames);
    }
    assert(err >= 0);
    monotonic = snd_pcm_hw_params_is_monotonic(params);
    can_pause = snd_pcm_hw_params_can_pause(params);
    err = snd_pcm_hw_params(handle, params);
    if (err < 0) {
        printf(("Unable to install hw params:"));
        snd_pcm_hw_params_dump(params, log);
        prg_exit(EXIT_FAILURE);
    }
    snd_pcm_hw_params_get_period_size(params, &chunk_size, 0);
    snd_pcm_hw_params_get_buffer_size(params, &buffer_size);
    if (chunk_size == buffer_size) {
        printf(("Can't use period equal to buffer size (%lu == %lu)"),
               chunk_size, buffer_size);
        prg_exit(EXIT_FAILURE);
    }
    err = snd_pcm_sw_params_current(handle, swparams);
    if (err < 0) {
        printf(("Unable to get current sw params."));
        prg_exit(EXIT_FAILURE);
    }
    if (avail_min < 0)
        n = chunk_size;
    else
        n = (double) rate * avail_min / 1000000;
    err = snd_pcm_sw_params_set_avail_min(handle, swparams, n);

    /* round up to closest transfer boundary */
    n = buffer_size;
    if (start_delay <= 0) {
        start_threshold = n + (double) rate * start_delay / 1000000;
    } else
        start_threshold = (double) rate * start_delay / 1000000;
    if (start_threshold < 1)
        start_threshold = 1;
    if (start_threshold > n)
        start_threshold = n;
    err = snd_pcm_sw_params_set_start_threshold(handle, swparams, start_threshold);
    assert(err >= 0);
    if (stop_delay <= 0)
        stop_threshold = buffer_size + (double) rate * stop_delay / 1000000;
    else
        stop_threshold = (double) rate * stop_delay / 1000000;
    err = snd_pcm_sw_params_set_stop_threshold(handle, swparams, stop_threshold);
    assert(err >= 0);

    if (snd_pcm_sw_params(handle, swparams) < 0) {
        printf(("unable to install sw params:"));
        snd_pcm_sw_params_dump(swparams, log);
        prg_exit(EXIT_FAILURE);
    }

    if (setup_chmap())
        prg_exit(EXIT_FAILURE);

    if (verbose)
        snd_pcm_dump(handle, log);

    bits_per_sample = snd_pcm_format_physical_width(hwparams.format);
    significant_bits_per_sample = snd_pcm_format_width(hwparams.format);
    bits_per_frame = bits_per_sample * hwparams.channels;
    chunk_bytes = chunk_size * bits_per_frame / 8;
    audiobuf = (u_char *)realloc(audiobuf, chunk_bytes);
    if (audiobuf == NULL) {
        printf(("not enough memory"));
        prg_exit(EXIT_FAILURE);
    }
    // fprintf(stderr, "real chunk_size = %i, frags = %i, total = %i\n", chunk_size, setup.buf.block.frags, setup.buf.block.frags * chunk_size);

    /* stereo VU-meter isn't always available... */
    if (vumeter == VUMETER_STEREO) {
        if (hwparams.channels != 2 || !interleaved || verbose > 2)
            vumeter = VUMETER_MONO;
    }

    /* show mmap buffer arragment */
    if (mmap_flag && verbose) {
        const snd_pcm_channel_area_t *areas;
        snd_pcm_uframes_t offset, size = chunk_size;
        int i;
        err = snd_pcm_mmap_begin(handle, &areas, &offset, &size);
        if (err < 0) {
            printf(("snd_pcm_mmap_begin problem: %s"), snd_strerror(err));
            prg_exit(EXIT_FAILURE);
        }
        for (i = 0; i < hwparams.channels; i++)
            fprintf(stderr, "mmap_area[%i] = %p,%u,%u (%u)\n", i, areas[i].addr, areas[i].first, areas[i].step, snd_pcm_format_physical_width(hwparams.format));
        /* not required, but for sure */
        snd_pcm_mmap_commit(handle, offset, 0);
    }

    buffer_frames = buffer_size;	/* for position test */
}

void LingmoUILoginSound::init_stdin(void)
{
    struct termios term;
    long flags;

    if (!interactive)
        return;
    if (!isatty(fileno(stdin))) {
        interactive = 0;
        return;
    }
    tcgetattr(fileno(stdin), &term);
    term_c_lflag = term.c_lflag;
    if (fd == fileno(stdin))
        return;
    flags = fcntl(fileno(stdin), F_GETFL);
    if (flags < 0 || fcntl(fileno(stdin), F_SETFL, flags|O_NONBLOCK) < 0)
        fprintf(stderr, ("stdin O_NONBLOCK flag setup failed\n"));
    term.c_lflag &= ~ICANON;
    tcsetattr(fileno(stdin), TCSANOW, &term);
}

void LingmoUILoginSound::done_stdin(void)
{
    struct termios term;

    if (!interactive)
        return;
    if (fd == fileno(stdin) || term_c_lflag == -1)
        return;
    tcgetattr(fileno(stdin), &term);
    term.c_lflag = term_c_lflag;
    tcsetattr(fileno(stdin), TCSANOW, &term);
}

void LingmoUILoginSound::do_pause(void)
{
    int err;
    unsigned char b;

    if (!can_pause) {
        fprintf(stderr, ("\rPAUSE command ignored (no hw support)\n"));
        return;
    }
    if (snd_pcm_state(handle) == SND_PCM_STATE_SUSPENDED)
        suspend();

    err = snd_pcm_pause(handle, 1);
    if (err < 0) {
        printf(("pause push error: %s"), snd_strerror(err));
        return;
    }
    while (1) {
        while (read(fileno(stdin), &b, 1) != 1);
        if (b == ' ' || b == '\r') {
            while (read(fileno(stdin), &b, 1) == 1);
            if (snd_pcm_state(handle) == SND_PCM_STATE_SUSPENDED)
                suspend();
            err = snd_pcm_pause(handle, 0);
            if (err < 0)
                printf(("pause release error: %s"), snd_strerror(err));
            return;
        }
    }
}

void LingmoUILoginSound::check_stdin(void)
{
    unsigned char b;

    if (!interactive)
        return;
    if (fd != fileno(stdin)) {
        while (read(fileno(stdin), &b, 1) == 1) {
            if (b == ' ' || b == '\r') {
                while (read(fileno(stdin), &b, 1) == 1);
                fprintf(stderr, ("\r=== PAUSE ===                                                            "));
                fflush(stderr);
                do_pause();
                fprintf(stderr, "                                                                          \r");
                fflush(stderr);
            }
        }
    }
}

#ifndef timersub
#define	timersub(a, b, result) \
    do { \
    (result)->tv_sec = (a)->tv_sec - (b)->tv_sec; \
    (result)->tv_usec = (a)->tv_usec - (b)->tv_usec; \
    if ((result)->tv_usec < 0) { \
    --(result)->tv_sec; \
    (result)->tv_usec += 1000000; \
    } \
    } while (0)
#endif

#ifndef timermsub
#define	timermsub(a, b, result) \
    do { \
    (result)->tv_sec = (a)->tv_sec - (b)->tv_sec; \
    (result)->tv_nsec = (a)->tv_nsec - (b)->tv_nsec; \
    if ((result)->tv_nsec < 0) { \
    --(result)->tv_sec; \
    (result)->tv_nsec += 1000000000L; \
    } \
    } while (0)
#endif

/* I/O error handler */
void LingmoUILoginSound::xrun(void)
{
    snd_pcm_status_t *status;
    int res;

    snd_pcm_status_alloca(&status);
    if ((res = snd_pcm_status(handle, status))<0) {
        printf(("status error: %s"), snd_strerror(res));
        prg_exit(EXIT_FAILURE);
    }
    if (snd_pcm_status_get_state(status) == SND_PCM_STATE_XRUN) {
        if (fatal_errors) {
            printf(("fatal %s: %s"),
                   stream == SND_PCM_STREAM_PLAYBACK ? ("underrun") : ("overrun"),
                   snd_strerror(res));
            prg_exit(EXIT_FAILURE);
        }
        if (monotonic) {
#ifdef HAVE_CLOCK_GETTIME
            struct timespec now, diff, tstamp;
            clock_gettime(CLOCK_MONOTONIC, &now);
            snd_pcm_status_get_trigger_htstamp(status, &tstamp);
            timermsub(&now, &tstamp, &diff);
            fprintf(stderr, _("%s!!! (at least %.3f ms long)\n"),
                    stream == SND_PCM_STREAM_PLAYBACK ? _("underrun") : _("overrun"),
                    diff.tv_sec * 1000 + diff.tv_nsec / 1000000.0);
#else
            fprintf(stderr, "%s !!!\n", ("underrun"));
#endif
        } else {
            struct timeval now, diff, tstamp;
            gettimeofday(&now, 0);
            snd_pcm_status_get_trigger_tstamp(status, &tstamp);
            timersub(&now, &tstamp, &diff);
            fprintf(stderr, ("%s!!! (at least %.3f ms long)\n"),
                    stream == SND_PCM_STREAM_PLAYBACK ? ("underrun") : ("overrun"),
                    diff.tv_sec * 1000 + diff.tv_usec / 1000.0);
        }
        if (verbose) {
            fprintf(stderr, ("Status:\n"));
            snd_pcm_status_dump(status, log);
        }
        if ((res = snd_pcm_prepare(handle))<0) {
            printf(("xrun: prepare error: %s"), snd_strerror(res));
            prg_exit(EXIT_FAILURE);
        }
        return;		/* ok, data should be accepted again */
    } if (snd_pcm_status_get_state(status) == SND_PCM_STATE_DRAINING) {
        if (verbose) {
            fprintf(stderr, ("Status(DRAINING):\n"));
            snd_pcm_status_dump(status, log);
        }
        if (stream == SND_PCM_STREAM_CAPTURE) {
            fprintf(stderr, ("capture stream format change? attempting recover...\n"));
            if ((res = snd_pcm_prepare(handle))<0) {
                printf(("xrun(DRAINING): prepare error: %s"), snd_strerror(res));
                prg_exit(EXIT_FAILURE);
            }
            return;
        }
    }
    if (verbose) {
        fprintf(stderr, ("Status(R/W):\n"));
        snd_pcm_status_dump(status, log);
    }
    printf(("read/write error, state = %s"), snd_pcm_state_name(snd_pcm_status_get_state(status)));
    prg_exit(EXIT_FAILURE);
}

/* I/O suspend handler */
void LingmoUILoginSound::suspend(void)
{
    int res;

    if (!quiet_mode) {
        fprintf(stderr, ("Suspended. Trying resume. ")); fflush(stderr);
    }
    while ((res = snd_pcm_resume(handle)) == -EAGAIN)
        sleep(1);	/* wait until suspend flag is released */
    if (res < 0) {
        if (!quiet_mode) {
            fprintf(stderr, ("Failed. Restarting stream. ")); fflush(stderr);
        }
        if ((res = snd_pcm_prepare(handle)) < 0) {
            printf(("suspend: prepare error: %s"), snd_strerror(res));
            prg_exit(EXIT_FAILURE);
        }
    }
    if (!quiet_mode)
        fprintf(stderr, ("Done.\n"));
}

void LingmoUILoginSound::print_vu_meter_mono(int perc, int maxperc)
{
    const int bar_length = 50;
    char line[80];
    int val;

    for (val = 0; val <= perc * bar_length / 100 && val < bar_length; val++)
        line[val] = '#';
    for (; val <= maxperc * bar_length / 100 && val < bar_length; val++)
        line[val] = ' ';
    line[val] = '+';
    for (++val; val <= bar_length; val++)
        line[val] = ' ';
    if (maxperc > 99)
        sprintf(line + val, "| MAX");
    else
        sprintf(line + val, "| %02i%%", maxperc);
    fputs(line, stderr);
    if (perc > 100)
        fprintf(stderr, (" !clip  "));
}

void LingmoUILoginSound::print_vu_meter_stereo(int *perc, int *maxperc)
{
    const int bar_length = 35;
    char line[80];
    int c;

    memset(line, ' ', sizeof(line) - 1);
    line[bar_length + 3] = '|';

    for (c = 0; c < 2; c++) {
        int p = perc[c] * bar_length / 100;
        char tmp[4];
        if (p > bar_length)
            p = bar_length;
        if (c)
            memset(line + bar_length + 6 + 1, '#', p);
        else
            memset(line + bar_length - p - 1, '#', p);
        p = maxperc[c] * bar_length / 100;
        if (p > bar_length)
            p = bar_length;
        if (c)
            line[bar_length + 6 + 1 + p] = '+';
        else
            line[bar_length - p - 1] = '+';
        if (ABS(maxperc[c]) > 99)
            sprintf(tmp, "MAX");
        else
            sprintf(tmp, "%02d%%", maxperc[c]);
        if (c)
            memcpy(line + bar_length + 3 + 1, tmp, 3);
        else
            memcpy(line + bar_length, tmp, 3);
    }
    line[bar_length * 2 + 6 + 2] = 0;
    fputs(line, stderr);
}

void LingmoUILoginSound::print_vu_meter(signed int *perc, signed int *maxperc)
{
    if (vumeter == VUMETER_STEREO)
        print_vu_meter_stereo(perc, maxperc);
    else
        print_vu_meter_mono(*perc, *maxperc);
}

/* peak handler */
void LingmoUILoginSound::compute_max_peak(u_char *data, size_t count)
{
    signed int val, max, perc[2], max_peak[2];
    static	int	run = 0;
    size_t ocount = count;
    int	format_little_endian = snd_pcm_format_little_endian(hwparams.format);
    int ichans, c;

    if (vumeter == VUMETER_STEREO)
        ichans = 2;
    else
        ichans = 1;

    memset(max_peak, 0, sizeof(max_peak));
    switch (bits_per_sample) {
    case 8: {
        signed char *valp = (signed char *)data;
        signed char mask = snd_pcm_format_silence(hwparams.format);
        c = 0;
        while (count-- > 0) {
            val = *valp++ ^ mask;
            val = abs(val);
            if (max_peak[c] < val)
                max_peak[c] = val;
            if (vumeter == VUMETER_STEREO)
                c = !c;
        }
        break;
    }
    case 16: {
        signed short *valp = (signed short *)data;
        signed short mask = snd_pcm_format_silence_16(hwparams.format);
        signed short sval;

        count /= 2;
        c = 0;
        while (count-- > 0) {
            if (format_little_endian)
                sval = le16toh(*valp);
            else
                sval = be16toh(*valp);
            sval = abs(sval) ^ mask;
            if (max_peak[c] < sval)
                max_peak[c] = sval;
            valp++;
            if (vumeter == VUMETER_STEREO)
                c = !c;
        }
        break;
    }
    case 24: {
        unsigned char *valp = data;
        signed int mask = snd_pcm_format_silence_32(hwparams.format);

        count /= 3;
        c = 0;
        while (count-- > 0) {
            if (format_little_endian) {
                val = valp[0] | (valp[1]<<8) | (valp[2]<<16);
            } else {
                val = (valp[0]<<16) | (valp[1]<<8) | valp[2];
            }
            /* Correct signed bit in 32-bit value */
            if (val & (1<<(bits_per_sample-1))) {
                val |= 0xff<<24;	/* Negate upper bits too */
            }
            val = abs(val) ^ mask;
            if (max_peak[c] < val)
                max_peak[c] = val;
            valp += 3;
            if (vumeter == VUMETER_STEREO)
                c = !c;
        }
        break;
    }
    case 32: {
        signed int *valp = (signed int *)data;
        signed int mask = snd_pcm_format_silence_32(hwparams.format);

        count /= 4;
        c = 0;
        while (count-- > 0) {
            if (format_little_endian)
                val = le32toh(*valp);
            else
                val = be32toh(*valp);
            val = abs(val) ^ mask;
            if (max_peak[c] < val)
                max_peak[c] = val;
            valp++;
            if (vumeter == VUMETER_STEREO)
                c = !c;
        }
        break;
    }
    default:
        if (run == 0) {
            fprintf(stderr, ("Unsupported bit size %d.\n"), (int)bits_per_sample);
            run = 1;
        }
        return;
    }
    max = 1 << (significant_bits_per_sample-1);
    if (max <= 0)
        max = 0x7fffffff;

    for (c = 0; c < ichans; c++) {
        if (bits_per_sample > 16)
            perc[c] = max_peak[c] / (max / 100);
        else
            perc[c] = max_peak[c] * 100 / max;
    }

    if (interleaved && verbose <= 2) {
        static int maxperc[2];
        static time_t t=0;
        const time_t tt=time(NULL);
        if(tt>t) {
            t=tt;
            maxperc[0] = 0;
            maxperc[1] = 0;
        }
        for (c = 0; c < ichans; c++)
            if (perc[c] > maxperc[c])
                maxperc[c] = perc[c];

        putc('\r', stderr);
        print_vu_meter(perc, maxperc);
        fflush(stderr);
    }
    else if(verbose==3) {
        fprintf(stderr, ("Max peak (%li samples): 0x%08x "), (long)ocount, max_peak[0]);
        for (val = 0; val < 20; val++)
            if (val <= perc[0] / 5)
                putc('#', stderr);
            else
                putc(' ', stderr);
        fprintf(stderr, " %i%%\n", perc[0]);
        fflush(stderr);
    }
}

void LingmoUILoginSound::do_test_position(void)
{
    static long counter = 0;
    static time_t tmr = -1;
    time_t now;
    static float availsum, delaysum, samples;
    static snd_pcm_sframes_t maxavail, maxdelay;
    static snd_pcm_sframes_t minavail, mindelay;
    static snd_pcm_sframes_t badavail = 0, baddelay = 0;
    snd_pcm_sframes_t outofrange;
    snd_pcm_sframes_t avail, delay;
    int err;

    err = snd_pcm_avail_delay(handle, &avail, &delay);
    if (err < 0)
        return;
    outofrange = (test_coef * (snd_pcm_sframes_t)buffer_frames) / 2;
    if (avail > outofrange || avail < -outofrange ||
        delay > outofrange || delay < -outofrange) {
        badavail = avail; baddelay = delay;
        availsum = delaysum = samples = 0;
        maxavail = maxdelay = 0;
        minavail = mindelay = buffer_frames * 16;
        fprintf(stderr, ("Suspicious buffer position (%li total): "
                         "avail = %li, delay = %li, buffer = %li\n"),
                ++counter, (long)avail, (long)delay, (long)buffer_frames);
    } else if (verbose) {
        time(&now);
        if (tmr == (time_t) -1) {
            tmr = now;
            availsum = delaysum = samples = 0;
            maxavail = maxdelay = 0;
            minavail = mindelay = buffer_frames * 16;
        }
        if (avail > maxavail)
            maxavail = avail;
        if (delay > maxdelay)
            maxdelay = delay;
        if (avail < minavail)
            minavail = avail;
        if (delay < mindelay)
            mindelay = delay;
        availsum += avail;
        delaysum += delay;
        samples++;
        if (avail != 0 && now != tmr) {
            fprintf(stderr, "BUFPOS: avg%li/%li "
                            "min%li/%li max%li/%li (%li) (%li:%li/%li)\n",
                    (long)(availsum / samples),
                    (long)(delaysum / samples),
                    (long)minavail, (long)mindelay,
                    (long)maxavail, (long)maxdelay,
                    (long)buffer_frames,
                    counter, badavail, baddelay);
            tmr = now;
        }
    }
}

/*
 */
#ifdef CONFIG_SUPPORT_CHMAP
u_char * LingmoUILoginSound::remap_data(u_char *data, size_t count)
{
    static u_char *tmp, *src, *dst;
    static size_t tmp_size;
    size_t sample_bytes = bits_per_sample / 8;
    size_t step = bits_per_frame / 8;
    size_t chunk_bytes;
    unsigned int ch, i;

    if (!hw_map)
        return data;

    chunk_bytes = count * bits_per_frame / 8;
    if (tmp_size < chunk_bytes) {
        free(tmp);
        tmp = (u_char *)malloc(chunk_bytes);
        if (!tmp) {
            printf(("not enough memory"));
            exit(1);
        }
        tmp_size = count;
    }

    src = data;
    dst = tmp;
    for (i = 0; i < count; i++) {
        for (ch = 0; ch < hwparams.channels; ch++) {
            memcpy(dst, src + sample_bytes * hw_map[ch],
                   sample_bytes);
            dst += sample_bytes;
        }
        src += step;
    }
    return tmp;
}

u_char ** LingmoUILoginSound::remap_datav(u_char **data, [[maybe_unused]] size_t count)
{
    static u_char **tmp;
    unsigned int ch;

    if (!hw_map)
        return data;

    if (!tmp) {
        tmp = (u_char **)malloc(sizeof(*tmp) * hwparams.channels);
        if (!tmp) {
            printf(("not enough memory"));
            exit(1);
        }
        for (ch = 0; ch < hwparams.channels; ch++)
            tmp[ch] = data[hw_map[ch]];
    }
    return tmp;
}
#else
#define remap_data(data, count)		(data)
#define remap_datav(data, count)	(data)
#endif

/*
 *  write function
 */

ssize_t LingmoUILoginSound::pcm_write(u_char *data, size_t count)
{
    ssize_t r;
    ssize_t result = 0;

    if (count < chunk_size) {
        snd_pcm_format_set_silence(hwparams.format, data + count * bits_per_frame / 8, (chunk_size - count) * hwparams.channels);
        count = chunk_size;
    }
    data = remap_data(data, count);
    while (count > 0 && !in_aborting) {
        if (test_position)
            do_test_position();
        check_stdin();
        r = writei_func(handle, data, count);
        if (test_position)
            do_test_position();
        if (r == -EAGAIN || (r >= 0 && (size_t)r < count)) {
            if (!test_nowait)
                snd_pcm_wait(handle, 100);
        } else if (r == -EPIPE) {
            xrun();
        } else if (r == -ESTRPIPE) {
            suspend();
        } else if (r < 0) {
            printf(("write error: %s"), snd_strerror(r));
            prg_exit(EXIT_FAILURE);
        }
        if (r > 0) {
            if (vumeter)
                compute_max_peak(data, r * hwparams.channels);
            result += r;
            count -= r;
            data += r * bits_per_frame / 8;
        }
    }
    return result;
}

ssize_t LingmoUILoginSound::pcm_writev(u_char **data, unsigned int channels, size_t count)
{
    ssize_t r;
    size_t result = 0;

    if (count != chunk_size) {
        unsigned int channel;
        size_t offset = count;
        size_t remaining = chunk_size - count;
        for (channel = 0; channel < channels; channel++)
            snd_pcm_format_set_silence(hwparams.format, data[channel] + offset * bits_per_sample / 8, remaining);
        count = chunk_size;
    }
    data = remap_datav(data, count);
    while (count > 0 && !in_aborting) {
        unsigned int channel;
        void *bufs[channels];
        size_t offset = result;
        for (channel = 0; channel < channels; channel++)
            bufs[channel] = data[channel] + offset * bits_per_sample / 8;
        if (test_position)
            do_test_position();
        check_stdin();
        r = writen_func(handle, bufs, count);
        if (test_position)
            do_test_position();
        if (r == -EAGAIN || (r >= 0 && (size_t)r < count)) {
            if (!test_nowait)
                snd_pcm_wait(handle, 100);
        } else if (r == -EPIPE) {
            xrun();
        } else if (r == -ESTRPIPE) {
            suspend();
        } else if (r < 0) {
            printf(("writev error: %s"), snd_strerror(r));
            prg_exit(EXIT_FAILURE);
        }
        if (r > 0) {
            if (vumeter) {
                for (channel = 0; channel < channels; channel++)
                    compute_max_peak(data[channel], r);
            }
            result += r;
            count -= r;
        }
    }
    return result;
}

/*
 *  read function
 */

ssize_t LingmoUILoginSound::pcm_read(u_char *data, size_t rcount)
{
    ssize_t r;
    size_t result = 0;
    size_t count = rcount;

    if (count != chunk_size) {
        count = chunk_size;
    }

    while (count > 0 && !in_aborting) {
        if (test_position)
            do_test_position();
        check_stdin();
        r = readi_func(handle, data, count);
        if (test_position)
            do_test_position();
        if (r == -EAGAIN || (r >= 0 && (size_t)r < count)) {
            if (!test_nowait)
                snd_pcm_wait(handle, 100);
        } else if (r == -EPIPE) {
            xrun();
        } else if (r == -ESTRPIPE) {
            suspend();
        } else if (r < 0) {
            printf(("read error: %s"), snd_strerror(r));
            prg_exit(EXIT_FAILURE);
        }
        if (r > 0) {
            if (vumeter)
                compute_max_peak(data, r * hwparams.channels);
            result += r;
            count -= r;
            data += r * bits_per_frame / 8;
        }
    }
    return rcount;
}

ssize_t LingmoUILoginSound::pcm_readv(u_char **data, unsigned int channels, size_t rcount)
{
    ssize_t r;
    size_t result = 0;
    size_t count = rcount;

    if (count != chunk_size) {
        count = chunk_size;
    }

    while (count > 0 && !in_aborting) {
        unsigned int channel;
        void *bufs[channels];
        size_t offset = result;
        for (channel = 0; channel < channels; channel++)
            bufs[channel] = data[channel] + offset * bits_per_sample / 8;
        if (test_position)
            do_test_position();
        check_stdin();
        r = readn_func(handle, bufs, count);
        if (test_position)
            do_test_position();
        if (r == -EAGAIN || (r >= 0 && (size_t)r < count)) {
            if (!test_nowait)
                snd_pcm_wait(handle, 100);
        } else if (r == -EPIPE) {
            xrun();
        } else if (r == -ESTRPIPE) {
            suspend();
        } else if (r < 0) {
            printf(("readv error: %s"), snd_strerror(r));
            prg_exit(EXIT_FAILURE);
        }
        if (r > 0) {
            if (vumeter) {
                for (channel = 0; channel < channels; channel++)
                    compute_max_peak(data[channel], r);
            }
            result += r;
            count -= r;
        }
    }
    return rcount;
}

/* setting the globals for playing raw data */
void LingmoUILoginSound::init_raw_data(void)
{
    hwparams = rhwparams;
}

/* calculate the data count to read from/to dsp */
off64_t LingmoUILoginSound::calc_count(void)
{
    off64_t count;

    if (timelimit == 0)
        if (sampleslimit == 0)
            count = pbrec_count;
        else
            count = snd_pcm_format_size(hwparams.format, sampleslimit * hwparams.channels);
    else {
        count = snd_pcm_format_size(hwparams.format, hwparams.rate * hwparams.channels);
        count *= (off64_t)timelimit;
    }
    return count < pbrec_count ? count : pbrec_count;
}

void LingmoUILoginSound::header(int rtype, char *name)
{
    if (!quiet_mode) {
        if (! name)
            name = (char *)((stream == SND_PCM_STREAM_PLAYBACK) ? "stdout" : "stdin");
        fprintf(stderr, "%s, ", snd_pcm_format_description(hwparams.format));
        fprintf(stderr, ("Rate %d Hz, "), hwparams.rate);
        if (hwparams.channels == 1)
            fprintf(stderr, ("Mono"));
        else if (hwparams.channels == 2)
            fprintf(stderr, ("Stereo"));
        else
            fprintf(stderr, ("Channels %i"), hwparams.channels);
        fprintf(stderr, "\n");
    }
}

/* playing raw data */

void LingmoUILoginSound::playback_go(int fd, size_t loaded, off64_t count, int rtype, char *name)
{
    int l, r;
    off64_t written = 0;
    off64_t c;

    header(rtype, name);
    set_params();

    while (loaded > chunk_bytes && written < count && !in_aborting) {
        if (pcm_write(audiobuf + written, chunk_size) <= 0)
            return;
        written += chunk_bytes;
        loaded -= chunk_bytes;
    }
    if (written > 0 && loaded > 0)
        memmove(audiobuf, audiobuf + written, loaded);

    l = loaded;
    while (written < count && !in_aborting) {
        do {
            c = count - written;
            if (c > chunk_bytes)
                c = chunk_bytes;

            /* c < l, there is more data loaded
             * then we actually need to write
             */
            if (c < l)
                l = c;

            c -= l;

            if (c == 0)
                break;
            r = safe_read(fd, audiobuf + l, c);
            if (r < 0) {
                perror(name);
                prg_exit(EXIT_FAILURE);
            }
            fdcount += r;
            if (r == 0)
                break;
            l += r;
        } while ((size_t)l < chunk_bytes);
        l = l * 8 / bits_per_frame;
        r = pcm_write(audiobuf, l);
        if (r != l)
            break;
        r = r * bits_per_frame / 8;
        written += r;
        l = 0;
    }
    snd_pcm_nonblock(handle, 0);
    snd_pcm_drain(handle);
    snd_pcm_nonblock(handle, nonblock);
}

int LingmoUILoginSound::read_header(int *loaded, int header_size)
{
    int ret;
    struct stat buf;

    ret = fstat(fd, &buf);
    if (ret < 0) {
        perror("fstat");
        prg_exit(EXIT_FAILURE);
    }

    /* don't be adventurous, get out if file size is smaller than
     * requested header size */
    if ((buf.st_mode & S_IFMT) == S_IFREG &&
            buf.st_size < header_size)
        return -1;

    if (*loaded < header_size) {
        header_size -= *loaded;
        ret = safe_read(fd, audiobuf + *loaded, header_size);
        if (ret != header_size) {
            printf(("read error"));
            prg_exit(EXIT_FAILURE);
        }
        *loaded += header_size;
    }
    return 0;
}

int LingmoUILoginSound::playback_wave(char *name, int *loaded)
{
    ssize_t dtawave;

    syslog (LOG_INFO, "start play %s file...", name);
    if (read_header(loaded, sizeof(WaveHeader)) < 0)
        return -1;

    if ((dtawave = test_wavefile(fd, audiobuf, *loaded)) < 0)
        return -1;

    pbrec_count = calc_count();
    playback_go(fd, dtawave, pbrec_count, FORMAT_WAVE, name);

    return 0;
}

int LingmoUILoginSound::playback_raw(char *name, int *loaded)
{
    syslog (LOG_INFO, "start play %s file...", name);

    init_raw_data();
    pbrec_count = calc_count();
    playback_go(fd, *loaded, pbrec_count, FORMAT_RAW, name);

    return 0;
}

/*
 *  let's play or capture it (capture_type says VOC/WAVE/raw)
 */

void LingmoUILoginSound::playback(char *name)
{
    int loaded = 0;

    pbrec_count = LLONG_MAX;
    fdcount = 0;
    if (!name || !strcmp(name, "-")) {
        fd = fileno(stdin);
        name = "stdin";
    } else {
        init_stdin();
        if ((fd = open(name, O_RDONLY, 0)) == -1) {
            perror(name);
            prg_exit(EXIT_FAILURE);
        }
    }

    /* parse the file header */
    if (playback_wave(name, &loaded) < 0)
        playback_raw(name, &loaded); /* should be raw data */


    if (fd != fileno(stdin))
        close(fd);
}

LingmoUILoginSound::LingmoUILoginSound()
{

}

int LingmoUILoginSound::wav_convert(int volume)
{
    if (volume > NORMAL_VOLUME) {
        volume = NORMAL_VOLUME;
    }

    float volume_factor = volume/NORMAL_VOLUME;
    QString cmd = "ffmpeg  -i ";
    cmd += PLAY_STARTUP_WAV;
    cmd += " ";
    cmd += QString("-filter:a volume=%1 ").arg(volume_factor);
    cmd += TMP_STARTUP_WAV_PATH;
    int err = system(cmd.toLatin1().data());
    if(err)
    {
        syslog(LOG_ERR, "file convert error, err:%d.", err);
        return -1;
    }

    return 0;
}

char* LingmoUILoginSound::checkPcm(char *pcmName)
{
    void **hints, **n;
    char *name, *defaultPcmName;

    defaultPcmName = "default";
    if (snd_device_name_hint(-1, "pcm", &hints) < 0)
        return "default";
    n = hints;

    /* If the configuration file does not specify or does not exist,
     * the PCM device list is traversed to find the last PCM device
     * as the output device to play the system sound effect. */
    while (*n != NULL) {
        name = snd_device_name_get_hint(*n, "NAME");
        if (strcmp(pcmName, name) == 0) {
            return pcmName;
        }
        else if (strstr(name, "sysdefault")) {
            defaultPcmName = name;
        }
        n++;
    }
    snd_device_name_free_hint(hints);

    if (name != NULL)
        free(name);

    syslog(LOG_INFO, "system does not have %s device, so the device from %s changed to %s device", pcmName, pcmName, defaultPcmName);
    return defaultPcmName;
}

bool LingmoUILoginSound::isSupportHardwareVolumeControl(char *card)
{
    int err;
    snd_hctl_t* handle;
    snd_hctl_elem_t* elem;
    snd_ctl_elem_info_t* info;
    snd_ctl_elem_id_t* id;
    snd_ctl_elem_info_alloca(&info);
    snd_ctl_elem_id_alloca(&id);
    bool ret = false;

    if ((err = snd_hctl_open(&handle, card, 0)) < 0) {
        syslog(LOG_ERR, "control %s open error: %s", card, snd_strerror(err));
        return ret;
    }

    if ((err = snd_hctl_load(handle)) < 0) {
        syslog(LOG_ERR, "control %s local error: %s", card, snd_strerror(err));
        return ret;
    }

    for (elem = snd_hctl_first_elem(handle); elem; elem = snd_hctl_elem_next(elem)) {
        if ((err = snd_hctl_elem_info(elem, info)) < 0) {
            syslog(LOG_ERR, "Control %s snd_hctl_elem_info error: %s", card, snd_strerror(err));
            return ret;
        }

        snd_hctl_elem_get_id(elem, id);
        char* str = snd_ctl_ascii_elem_id_get(id);

        if (strstr(str, "Volume")) {
            ret = true;
            break;
        }
    }

    syslog(LOG_INFO, "card %s %s hardware volume control.", card, ret ? "support" : "unsupport");
    return ret;
}
