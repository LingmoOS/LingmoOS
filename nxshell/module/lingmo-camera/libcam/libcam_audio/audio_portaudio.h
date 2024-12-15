/*******************************************************************************#
#           guvcview              http://guvcview.sourceforge.net               #
#                                                                               #
#           Paulo Assis <pj.assis@gmail.com>                                    #
#           Nobuhiro Iwamatsu <iwamatsu@nigauri.org>                            #
#                             Add UYVY color support(Macbook iSight)            #
#           Flemming Frandsen <dren.dk@gmail.com>                               #
#                             Add VU meter OSD                                  #
#                                                                               #
# This program is free software; you can redistribute it and/or modify          #
# it under the terms of the GNU General Public License as published by          #
# the Free Software Foundation; either version 2 of the License, or             #
# (at your option) any later version.                                           #
#                                                                               #
# This program is distributed in the hope that it will be useful,               #
# but WITHOUT ANY WARRANTY; without even the implied warranty of                #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                 #
# GNU General Public License for more details.                                  #
#                                                                               #
# You should have received a copy of the GNU General Public License             #
# along with this program; if not, write to the Free Software                   #
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA     #
#                                                                               #
********************************************************************************/

#ifndef GVIEWAUDIO_PORTAUDIO_H
#define GVIEWAUDIO_PORTAUDIO_H

/*
 * init portaudio api
 * args:
 *    audio_ctx - pointer to audio context data
 *
 * asserts:
 *    audio_ctx is not null
 *
 * returns: error code (0 = E_OK)
 */
int audio_init_portaudio(audio_context_t *audio_ctx);

/*
 * set audio device
 * args:
 *   audio_ctx - pointer to audio context data
 *   index - device index to set
 *
 * asserts:
 *   audio_ctx is not null
 *
 * returns: none
 */
void audio_set_portaudio_device(audio_context_t *audio_ctx, int index);

/*
 * start portaudio stream capture
 * args:
 *   audio_ctx - pointer to audio context data
 *
 * asserts:
 *   audio_ctx is not null
 *
 * returns: error code
 */
int audio_start_portaudio(audio_context_t *audio_ctx);

/*
 * stop portaudio stream capture
 * args:
 *   audio_ctx - pointer to audio context data
 *
 * asserts:
 *   audio_ctx is not null
 *
 * returns: error code
 */
int audio_stop_portaudio(audio_context_t *audio_ctx);

/*
 * close and clean audio context for portaudio api
 * args:
 *   audio_ctx - pointer to audio context data
 *
 * asserts:
 *   none
 *
 * returns: none
 */
void audio_close_portaudio(audio_context_t *audio_ctx);

void set_audio_codec_ind(int index);
#endif
