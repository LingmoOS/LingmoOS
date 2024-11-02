/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#ifndef APLAYPLAY_H
#define APLAYPLAY_H

//#include "aconfig.h"
#include <stdio.h>
#include <malloc.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <time.h>
#include <locale.h>
#include <alsa/asoundlib.h>
#include <assert.h>
#include <termios.h>
#include <signal.h>
#include <poll.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <endian.h>
#include "formats.h"

#include <QDebug>
#include <QObject>
#include <QProcess>
#include <syslog.h>

#define BUFFER_LEN 1024
//开机音乐播放路径
#define PLAY_STARTUP_WAV "/usr/share/lingmo/lingmo-session-manager/startup.wav"
#define TMP_STARTUP_WAV_PATH "/tmp/startup.wav"
#define NORMAL_VOLUME 100.0
#define MIN_VOLUME 0.0
#define ABS(a)  (a) < 0 ? -(a) : (a)

#ifdef SND_CHMAP_API_VERSION
#define CONFIG_SUPPORT_CHMAP	1
#endif

#ifndef LLONG_MAX
#define LLONG_MAX    9223372036854775807LL
#endif

#ifndef le16toh
#include <asm/byteorder.h>
#define le16toh(x) __le16_to_cpu(x)
#define be16toh(x) __be16_to_cpu(x)
#define le32toh(x) __le32_to_cpu(x)
#define be32toh(x) __be32_to_cpu(x)
#endif

#define DEFAULT_FORMAT		SND_PCM_FORMAT_U8
#define DEFAULT_SPEED 		8000

#define FORMAT_DEFAULT		-1
#define FORMAT_RAW		0
#define FORMAT_VOC		1
#define FORMAT_WAVE		2
#define FORMAT_AU		3

/* global data */

static snd_pcm_sframes_t (*readi_func)(snd_pcm_t *handle, void *buffer, snd_pcm_uframes_t size);
static snd_pcm_sframes_t (*writei_func)(snd_pcm_t *handle, const void *buffer, snd_pcm_uframes_t size);
static snd_pcm_sframes_t (*readn_func)(snd_pcm_t *handle, void **bufs, snd_pcm_uframes_t size);
static snd_pcm_sframes_t (*writen_func)(snd_pcm_t *handle, void **bufs, snd_pcm_uframes_t size);

enum {
    VUMETER_NONE,
    VUMETER_MONO,
    VUMETER_STEREO
};

static snd_pcm_t *handle;
static struct {
    snd_pcm_format_t format;
    unsigned int channels;
    unsigned int rate;
} hwparams, rhwparams;
static int timelimit = 0;
static int sampleslimit = 0;
static int quiet_mode = 0;
static int file_type = FORMAT_DEFAULT;
static int open_mode = 0;
static snd_pcm_stream_t stream = SND_PCM_STREAM_PLAYBACK;
static int mmap_flag = 0;
static int interleaved = 1;
static int nonblock = 0;
static volatile sig_atomic_t in_aborting = 0;
static u_char *audiobuf = NULL;
static snd_pcm_uframes_t chunk_size = 0;
static unsigned period_time = 0;
static unsigned buffer_time = 0;
static snd_pcm_uframes_t period_frames = 0;
static snd_pcm_uframes_t buffer_frames = 0;
static int avail_min = -1;
static int start_delay = 0;
static int stop_delay = 0;
static int monotonic = 0;
static int interactive = 0;
static int can_pause = 0;
static int fatal_errors = 0;
static int verbose = 0;
static int vumeter = VUMETER_NONE;
static int buffer_pos = 0;
static size_t significant_bits_per_sample, bits_per_sample, bits_per_frame;
static size_t chunk_bytes;
static int test_position = 0;
static int test_coef = 8;
static int test_nowait = 0;
static snd_output_t *log;
static long long max_file_size = 0;
static int max_file_time = 0;
static int use_strftime = 0;
volatile static int recycle_capture_file = 0;
static long term_c_lflag = -1;
static int dump_hw_params = 0;

static int fd = -1;
static off64_t pbrec_count = LLONG_MAX, fdcount;
static int vocmajor, vocminor;

static char *pidfile_name = NULL;
//FILE *pidf = NULL;
static int pidfile_written = 0;

#ifdef CONFIG_SUPPORT_CHMAP
static snd_pcm_chmap_t *channel_map = NULL; /* chmap to override */
static unsigned int *hw_map = NULL; /* chmap to follow */
#endif
//extern "C"{
/* needed prototypes */

class LingmoUILoginSound
{
public:
    static LingmoUILoginSound& getInstance();
    int aplayMain(char *filename,char *playdev);
    char* checkPcm(char *name);
    bool isSupportHardwareVolumeControl(char *name);
    int wav_convert(int volume);

 private:
    LingmoUILoginSound();
    LingmoUILoginSound(const LingmoUILoginSound&) = delete;
    LingmoUILoginSound(LingmoUILoginSound&&) = delete;
    LingmoUILoginSound operator=(const LingmoUILoginSound&) = delete;
    LingmoUILoginSound operator=(LingmoUILoginSound&&) = delete;
    ~LingmoUILoginSound() = default;

    static void check_wavefile_space(u_char *buffer,u_int len, size_t blimit);
    static void device_list();
    static void pcm_list(void);
    static void prg_exit(int code);
    static void signal_handler(int sig);
    static void signal_handler_recycle (int sig);

    static ssize_t safe_read(int fd, void *buf, size_t count);

    static size_t test_wavefile_read(int fd, u_char *buffer, size_t *size, size_t reqsize, int line);
    static ssize_t test_wavefile(int fd, u_char *_buffer, size_t size);

    static void show_available_sample_formats(snd_pcm_hw_params_t* params);
    static int setup_chmap(void);
    static void set_params(void);
    static void init_stdin(void);
    static void done_stdin(void);
    static void do_pause(void);
    static void check_stdin(void);
    static void xrun(void);

    static void suspend(void);
    static void print_vu_meter_mono(int perc, int maxperc);
    static void print_vu_meter_stereo(int *perc, int *maxperc);
    static void print_vu_meter(signed int *perc, signed int *maxperc);
    static void compute_max_peak(u_char *data, size_t count);
    static void do_test_position(void);
    static u_char *remap_data(u_char *data, size_t count);
    static u_char **remap_datav(u_char **data, size_t count);

    static ssize_t pcm_write(u_char *data, size_t count);
    static ssize_t pcm_writev(u_char **data, unsigned int channels, size_t count);
    static ssize_t pcm_read(u_char *data, size_t rcount);
    static ssize_t pcm_readv(u_char **data, unsigned int channels, size_t rcount);
    static ssize_t voc_pcm_write(u_char *data, size_t count);

    static void init_raw_data(void);
    static off64_t calc_count(void);

    static void header(int rtype, char *name);
    static void playback_go(int fd, size_t loaded, off64_t count, int rtype, char *name);
    static int read_header(int *loaded, int header_size);
    static int playback_wave(char *name, int *loaded);
    static int playback_raw(char *name, int *loaded);
    static void playback(char *name);

};

#endif // APLAYPLAY_H
