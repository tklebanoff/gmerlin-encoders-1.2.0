/*****************************************************************
 * gmerlin-encoders - encoder plugins for gmerlin
 *
 * Copyright (c) 2001 - 2011 Members of the Gmerlin project
 * gmerlin-general@lists.sourceforge.net
 * http://gmerlin.sourceforge.net
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * *****************************************************************/


#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <config.h>
#include <gmerlin/translation.h>

#include <gmerlin_encoders.h>

#include <gmerlin/plugin.h>
#include <gmerlin/utils.h>
#include <ogg/ogg.h>
#include "ogg_common.h"

extern const bg_ogg_codec_t bg_flacogg_codec;

static const bg_parameter_info_t * get_audio_parameters_flacogg(void * data)
  {
  return bg_flacogg_codec.get_parameters();
  }


static int
add_audio_stream_flacogg(void * data,
                         const gavl_metadata_t * metadata,
                         const gavl_audio_format_t * format)
  {
  int ret;
  ret = bg_ogg_encoder_add_audio_stream(data, format);
  bg_ogg_encoder_init_audio_stream(data, ret, &bg_flacogg_codec);
  return ret;
  }

static int
open_flacogg(void * data, const char * file,
             const gavl_metadata_t * metadata,
             const bg_chapter_list_t * chapter_list)
  {
  return bg_ogg_encoder_open(data, file, metadata, chapter_list,
                             "ogg");
  }


const bg_encoder_plugin_t the_plugin =
  {
    .common =
    {
      BG_LOCALE,
      .name =            "e_flacogg",       /* Unique short name */
      .long_name =       TRS("Flac in Ogg encoder"),
      .description =     TRS("Encoder for flac streams in Ogg containers. Based on libflac (http://flac.sourceforge.net)"),
      .type =            BG_PLUGIN_ENCODER_AUDIO,
      .flags =           BG_PLUGIN_FILE,
      .priority =        5,
      .create =            bg_ogg_encoder_create,
      .destroy =           bg_ogg_encoder_destroy,
#if 0
      .get_parameters =    get_parameters_flacogg,
      .set_parameter =     set_parameter_flacogg,
#endif
    },
    .max_audio_streams =   1,
    .max_video_streams =   0,
    
    .set_callbacks =       bg_ogg_encoder_set_callbacks,
    
    .open =                open_flacogg,
    
    .get_audio_parameters =    get_audio_parameters_flacogg,

    .add_audio_stream =        add_audio_stream_flacogg,
    
    .set_audio_parameter =     bg_ogg_encoder_set_audio_parameter,

    .start =                  bg_ogg_encoder_start,

    .get_audio_format =        bg_ogg_encoder_get_audio_format,
    
    .write_audio_frame =   bg_ogg_encoder_write_audio_frame,
    .close =               bg_ogg_encoder_close,
  };

/* Include this into all plugin modules exactly once
   to let the plugin loader obtain the API version */
BG_GET_PLUGIN_API_VERSION;
