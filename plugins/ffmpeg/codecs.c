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

#include "ffmpeg_common.h"
#include "params.h"
#include <gmerlin/utils.h>
#include <gmerlin/translation.h>
#include <gmerlin/log.h>

#define LOG_DOMAIN "ffmpeg.codecs"

#define ENCODE_PARAM_MP2                 \
  {                                      \
    .name =      "ff_bit_rate_str",        \
    .long_name = TRS("Bit rate (kbps)"),        \
    .type =      BG_PARAMETER_STRINGLIST, \
    .val_default = { .val_str = "128" },       \
    .multi_names = (char const *[]){ "32",  "48", "56", "64", "80", "96", "112", \
                   "128", "160", "192", "224", "256", "320", "384",\
                   NULL } \
  },

#define ENCODE_PARAM_MP3                 \
  {                                      \
    .name =      "ff_bit_rate_str",        \
    .long_name = TRS("Bit rate (kbps)"),        \
    .type =      BG_PARAMETER_STRINGLIST, \
    .val_default = { .val_str = "128" },       \
    .multi_names = (char const *[]){ "32", "40", "48", "56", "64", "80", "96", \
                   "112", "128", "160", "192", "224", "256", "320",\
                   NULL } \
  },

#define ENCODE_PARAM_AC3 \
  {                                      \
    .name =      "ff_bit_rate_str",        \
    .long_name = TRS("Bit rate (kbps)"),        \
    .type =      BG_PARAMETER_STRINGLIST, \
    .val_default = { .val_str = "128" },       \
    .multi_names = (char const *[]){ "32", "40", "48", "56", "64", "80", "96", "112", "128", \
                   "160", "192", "224", "256", "320", "384", "448", "512", \
                   "576", "640", NULL } \
  },

#define ENCODE_PARAM_WMA \
  {                                      \
    .name =      "ff_bit_rate_str",        \
    .long_name = TRS("Bit rate (kbps)"),        \
    .type =      BG_PARAMETER_STRINGLIST, \
    .val_default = { .val_str = "128" },       \
    .multi_names = (char const *[]){ "24", "48", "64", "96", "128", NULL } \
  },
    
    
#define ENCODE_PARAM_VIDEO_RATECONTROL \
  {                                           \
    .name =      "rate_control",                       \
    .long_name = TRS("Rate control"),                     \
    .type =      BG_PARAMETER_SECTION,         \
  },                                        \
    PARAM_BITRATE_VIDEO,                    \
    PARAM_BITRATE_TOLERANCE,                \
    PARAM_RC_MIN_RATE,                      \
    PARAM_RC_MAX_RATE,                      \
    PARAM_RC_BUFFER_SIZE,                   \
    PARAM_RC_INITIAL_COMPLEX,               \
    PARAM_RC_INITIAL_BUFFER_OCCUPANCY

#define ENCODE_PARAM_VIDEO_QUANTIZER_I \
  {                                           \
    .name =      "quantizer",                       \
    .long_name = TRS("Quantizer"),                     \
    .type =      BG_PARAMETER_SECTION,         \
  },                                        \
    PARAM_QMIN,                             \
    PARAM_QMAX,                             \
    PARAM_MAX_QDIFF,                        \
    PARAM_FLAG_QSCALE,                      \
    PARAM_QSCALE,                      \
    PARAM_QCOMPRESS,                        \
    PARAM_QBLUR,                            \
    PARAM_QUANTIZER_NOISE_SHAPING,          \
    PARAM_TRELLIS

#define ENCODE_PARAM_VIDEO_QUANTIZER_IP \
  ENCODE_PARAM_VIDEO_QUANTIZER_I,               \
  PARAM_I_QUANT_FACTOR,                         \
  PARAM_I_QUANT_OFFSET

#define ENCODE_PARAM_VIDEO_QUANTIZER_IPB \
  ENCODE_PARAM_VIDEO_QUANTIZER_IP,        \
  PARAM_B_QUANT_FACTOR,                   \
  PARAM_B_QUANT_OFFSET

#define ENCODE_PARAM_VIDEO_FRAMETYPES_IP \
  {                                           \
    .name =      "frame_types",                       \
    .long_name = TRS("Frame types"),                     \
    .type =      BG_PARAMETER_SECTION,         \
  },                                        \
  PARAM_GOP_SIZE,                      \
  PARAM_SCENE_CHANGE_THRESHOLD,       \
  PARAM_SCENECHANGE_FACTOR,        \
  PARAM_FLAG_CLOSED_GOP,         \
  PARAM_FLAG2_STRICT_GOP

#define ENCODE_PARAM_VIDEO_FRAMETYPES_IPB \
  ENCODE_PARAM_VIDEO_FRAMETYPES_IP,        \
  PARAM_MAX_B_FRAMES,                 \
  PARAM_B_FRAME_STRATEGY

#define ENCODE_PARAM_VIDEO_ME \
  {                                           \
    .name =      "motion_estimation",                       \
    .long_name = TRS("Motion estimation"),                     \
    .type =      BG_PARAMETER_SECTION,         \
  },                                        \
    PARAM_ME_METHOD,                        \
    PARAM_ME_CMP,\
    PARAM_ME_CMP_CHROMA,\
    PARAM_ME_RANGE,\
    PARAM_ME_THRESHOLD,\
    PARAM_MB_DECISION,\
    PARAM_DIA_SIZE

#define ENCODE_PARAM_VIDEO_ME_PRE \
  {                                           \
    .name =      "motion_estimation",                       \
    .long_name = TRS("ME pre-pass"),                     \
    .type =      BG_PARAMETER_SECTION,         \
  },                                        \
    PARAM_PRE_ME,\
    PARAM_ME_PRE_CMP,\
    PARAM_ME_PRE_CMP_CHROMA,\
    PARAM_PRE_DIA_SIZE

#define ENCODE_PARAM_VIDEO_QPEL                 \
  {                                           \
    .name =      "qpel_motion_estimation",                       \
    .long_name = TRS("Qpel ME"),                     \
    .type =      BG_PARAMETER_SECTION,         \
  },                                        \
    PARAM_FLAG_QPEL, \
    PARAM_ME_SUB_CMP,\
    PARAM_ME_SUB_CMP_CHROMA,\
    PARAM_ME_SUBPEL_QUALITY

#define ENCODE_PARAM_VIDEO_MASKING \
  {                                \
    .name =      "masking",                       \
    .long_name = TRS("Masking"),                     \
    .type =      BG_PARAMETER_SECTION,         \
  },                                        \
    PARAM_LUMI_MASKING, \
    PARAM_DARK_MASKING, \
    PARAM_TEMPORAL_CPLX_MASKING, \
    PARAM_SPATIAL_CPLX_MASKING, \
    PARAM_BORDER_MASKING, \
    PARAM_P_MASKING,                                       \
    PARAM_FLAG_NORMALIZE_AQP

#define ENCODE_PARAM_VIDEO_MISC \
  {                                           \
    .name =      "misc",                       \
    .long_name = TRS("Misc"),                     \
    .type =      BG_PARAMETER_SECTION,         \
  },                                        \
    PARAM_STRICT_STANDARD_COMPLIANCE,       \
    PARAM_NOISE_REDUCTION, \
    PARAM_FLAG_GRAY, \
    PARAM_FLAG_BITEXACT

static const bg_parameter_info_t parameters_mpeg4[] = {
  ENCODE_PARAM_VIDEO_FRAMETYPES_IPB,
  PARAM_FLAG_AC_PRED_MPEG4,
  ENCODE_PARAM_VIDEO_RATECONTROL,
  ENCODE_PARAM_VIDEO_QUANTIZER_IPB,
  PARAM_FLAG_CBP_RD,
  ENCODE_PARAM_VIDEO_ME,
  PARAM_FLAG_GMC,
  PARAM_FLAG_4MV,
  PARAM_FLAG_MV0,
  PARAM_FLAG_QP_RD,
  ENCODE_PARAM_VIDEO_ME_PRE,
  ENCODE_PARAM_VIDEO_QPEL,
  ENCODE_PARAM_VIDEO_MASKING,
  ENCODE_PARAM_VIDEO_MISC,
  { /* End of parameters */ }
};

static const bg_parameter_info_t parameters_mjpeg[] = {
  ENCODE_PARAM_VIDEO_RATECONTROL,
  ENCODE_PARAM_VIDEO_QUANTIZER_I,
  ENCODE_PARAM_VIDEO_MISC,
  { /* End of parameters */ }
};

static const bg_parameter_info_t parameters_mpeg1[] = {
  ENCODE_PARAM_VIDEO_FRAMETYPES_IPB,
  ENCODE_PARAM_VIDEO_RATECONTROL,
  ENCODE_PARAM_VIDEO_QUANTIZER_IPB,
  ENCODE_PARAM_VIDEO_ME,
  ENCODE_PARAM_VIDEO_ME_PRE,
  ENCODE_PARAM_VIDEO_MASKING,
  ENCODE_PARAM_VIDEO_MISC,
  { /* End of parameters */ }
};


static const bg_parameter_info_t parameters_msmpeg4v3[] = {
  ENCODE_PARAM_VIDEO_FRAMETYPES_IP,
  ENCODE_PARAM_VIDEO_RATECONTROL,
  ENCODE_PARAM_VIDEO_QUANTIZER_IP,
  ENCODE_PARAM_VIDEO_ME,
  ENCODE_PARAM_VIDEO_ME_PRE,
  ENCODE_PARAM_VIDEO_MASKING,
  ENCODE_PARAM_VIDEO_MISC,
  { /* End of parameters */ }
};

static const bg_parameter_info_t parameters_libx264[] = {
  {
    .name =      "libx264_preset",
    .long_name = TRS("Preset"),
    .type =      BG_PARAMETER_STRINGLIST,
    .val_default = {val_str: "medium"},
    .multi_names = (char const *[]){"ultrafast",
                                    "superfast",
                                    "veryfast",
                                    "faster",
                                    "fast",
                                    "medium",
                                    "slow",
                                    "slower",
                                    "veryslow",
                                    "placebo",
                                    (char *)0},
    .multi_labels = (char const *[]){TRS("Ultrafast"),
                                     TRS("Superfast"),
                                     TRS("Veryfast"),
                                     TRS("Faster"),
                                     TRS("Fast"),
                                     TRS("Medium"),
                                     TRS("Slow"),
                                     TRS("Slower"),
                                     TRS("Veryslow"),
                                     TRS("Placebo"),
                                     (char *)0 },
  },
  {
    .name =      "libx264_tune",
    .long_name = TRS("Tune"),
    .type =      BG_PARAMETER_STRINGLIST,
    .val_default = {val_str: "$none"},
    .multi_names = (char const *[]){"$none",
                                    "film",
                                    "animation",
                                    "grain",
                                    "stillimage",
                                    "psnr",
                                    "ssim",
                                    "fastdecode",
                                    "zerolatency",
                                    (char *)0},
    .multi_labels = (char const *[]){TRS("None"),
                                     TRS("Film"),
                                     TRS("Animation"),
                                     TRS("Grain"),
                                     TRS("Still image"),
                                     TRS("PSNR"),
                                     TRS("SSIM"),
                                     TRS("Fast decode"),
                                     TRS("Zero latency"),
                                     (char *)0},
  },
  {
    .name =      "ff_bit_rate_video",
    .long_name = TRS("Bit rate (kbps)"),
    .type =      BG_PARAMETER_INT,
    .val_min     = { .val_i = 0 },
    .val_max     = { .val_i = 10000 },
    .val_default = { .val_i = 0 },
    .help_string = TRS("If > 0 encode with average bitrate"),
  },
  {
    .name =      "libx264_crf",
    .long_name = TRS("Quality-based VBR"),
    .type =      BG_PARAMETER_SLIDER_FLOAT,
    .val_min     = { .val_f = -1.0 },
    .val_max     = { .val_f = 51.0 },
    .val_default = { .val_f = 23.0 },
    .help_string = TRS("Negative means disable"),
    .num_digits  = 2,
  },
  {
    .name =      "libx264_qp",
    .long_name = TRS("Force constant QP"),
    .type =      BG_PARAMETER_SLIDER_INT,
    .val_min     = { .val_i = -1 },
    .val_max     = { .val_i = 69 },
    .val_default = { .val_i = -1 },
    .help_string = TRS("Negative means disable, 0 means lossless"),
  },
  { /* End */ },
};

/* Audio */

static const bg_parameter_info_t parameters_ac3[] = {
  ENCODE_PARAM_AC3
  { /* End of parameters */ }
};

static const bg_parameter_info_t parameters_mp2[] = {
  ENCODE_PARAM_MP2
  { /* End of parameters */ }
};

static const bg_parameter_info_t parameters_mp3[] = {
  ENCODE_PARAM_MP3
  { /* End of parameters */ }
};

static const bg_parameter_info_t parameters_wma[] = {
  ENCODE_PARAM_WMA
  { /* End of parameters */ }
};

static const ffmpeg_codec_info_t audio_codecs[] =
  {
    {
      .name      = "pcm_s16be",
      .long_name = TRS("16 bit PCM"),
      .id        = CODEC_ID_PCM_S16BE,
    },
    {
      .name      = "pcm_s16le",
      .long_name = TRS("16 bit PCM"),
      .id        = CODEC_ID_PCM_S16LE,
    },
    {
      .name      = "pcm_s8",
      .long_name = TRS("8 bit PCM"),
      .id        = CODEC_ID_PCM_S8,
    },
    {
      .name      = "pcm_u8",
      .long_name = TRS("8 bit PCM"),
      .id        = CODEC_ID_PCM_U8,
    },
    {
      .name      = "pcm_alaw",
      .long_name = TRS("alaw"),
      .id        = CODEC_ID_PCM_ALAW,
    },
    {
      .name      = "pcm_mulaw",
      .long_name = TRS("mulaw"),
      .id        = CODEC_ID_PCM_MULAW,
    },
    {
      .name      = "ac3",
      .long_name = TRS("AC3"),
      .id        = CODEC_ID_AC3,
      .parameters = parameters_ac3,
    },
    {
      .name      = "mp2",
      .long_name = TRS("MPEG audio layer 2"),
      .id        = CODEC_ID_MP2,
      .parameters = parameters_mp2,
    },
    {
      .name      = "wma1",
      .long_name = TRS("Windows media Audio 1"),
      .id        = CODEC_ID_WMAV1,
      .parameters = parameters_wma,
    },
    {
      .name      = "wma2",
      .long_name = TRS("Windows media Audio 2"),
      .id        = CODEC_ID_WMAV2,
      .parameters = parameters_wma,
    },
    {
      .name      = "mp3",
      .long_name = TRS("MPEG audio layer 3"),
      .id        = CODEC_ID_MP3,
      .parameters = parameters_mp3,
    },
    { /* End of array */ }
  };

static const ffmpeg_codec_info_t video_codecs[] =
  {
    {
      .name       = "mjpeg",
      .long_name  = TRS("Motion JPEG"),
      .id         = CODEC_ID_MJPEG,
      .parameters = parameters_mjpeg,
      .pixelformats = (enum PixelFormat[]) { PIX_FMT_YUVJ420P, PIX_FMT_NB },
    },
    {
      .name       = "mpeg4",
      .long_name  = TRS("MPEG-4"),
      .id         = CODEC_ID_MPEG4,
      .parameters = parameters_mpeg4,
      .pixelformats = (enum PixelFormat[]) { PIX_FMT_YUV420P, PIX_FMT_NB },
    },
    {
      .name       = "msmpeg4v3",
      .long_name  = TRS("Divx 3 compatible"),
      .id         = CODEC_ID_MSMPEG4V3,
      .parameters = parameters_msmpeg4v3,
      .pixelformats = (enum PixelFormat[]) { PIX_FMT_YUV420P, PIX_FMT_NB },
    },
    {
      .name       = "mpeg1video",
      .long_name  = TRS("MPEG-1 Video"),
      .id         = CODEC_ID_MPEG1VIDEO,
      .parameters = parameters_mpeg1,
      .pixelformats = (enum PixelFormat[]) { PIX_FMT_YUV420P, PIX_FMT_NB },
    },
    {
      .name       = "mpeg2video",
      .long_name  = TRS("MPEG-2 Video"),
      .id         = CODEC_ID_MPEG2VIDEO,
      .parameters = parameters_mpeg1,
      .pixelformats = (enum PixelFormat[]) { PIX_FMT_YUV420P, PIX_FMT_NB },
    },
    {
      .name       = "flv1",
      .long_name  = TRS("Flash 1"),
      .id         = CODEC_ID_FLV1,
      .parameters = parameters_msmpeg4v3,
      .pixelformats = (enum PixelFormat[]) { PIX_FMT_YUV420P, PIX_FMT_NB },
    },
    {
      .name       = "wmv1",
      .long_name  = TRS("WMV 1"),
      .id         = CODEC_ID_WMV1,
      .parameters = parameters_msmpeg4v3,
      .pixelformats = (enum PixelFormat[]) { PIX_FMT_YUV420P, PIX_FMT_NB },
    },
    {
      .name       = "rv10",
      .long_name  = TRS("Real Video 1"),
      .id         = CODEC_ID_RV10,
      .parameters = parameters_msmpeg4v3,
      .pixelformats = (enum PixelFormat[]) { PIX_FMT_YUV420P, PIX_FMT_NB },
    },
    {
      .name       = "libx264",
      .long_name  = TRS("H.264"),
      .id         = CODEC_ID_H264,
      .parameters = parameters_libx264,
      .pixelformats = (enum PixelFormat[]) { PIX_FMT_YUV420P, PIX_FMT_NB },
    },
#if 0
    {
      .name       = "wmv2",
      .long_name  = TRS("WMV 2"),
      .id         = CODEC_ID_WMV2,
      .parameters = parameters_msmpeg4v3
    },
#endif
    { /* End of array */ }
  };

static const ffmpeg_codec_info_t **
add_codec_info(const ffmpeg_codec_info_t ** info, enum CodecID id, int * num)
  {
  int i;
  /* Check if the codec id is already in the array */

  for(i = 0; i < *num; i++)
    {
    if(info[i]->id == id)
      return info;
    }

  info = realloc(info, ((*num)+1) * sizeof(*info));
  
  info[*num] = NULL;

  i = 0;
  while(audio_codecs[i].name)
    {
    if(audio_codecs[i].id == id)
      {
      info[*num] = &audio_codecs[i];
      break;
      }
    i++;
    }

  if(!info[*num])
    {
    i = 0;
    while(video_codecs[i].name)
      {
      if(video_codecs[i].id == id)
        {
        info[*num] = &video_codecs[i];
        break;
        }
      i++;
      }
    }
  (*num)++;
  return info;
  }

static const bg_parameter_info_t audio_parameters[] =
  {
    {
      .name      = "codec",
      .long_name = TRS("Codec"),
      .type      = BG_PARAMETER_MULTI_MENU,
    },
    { /* */ }
  };

static const bg_parameter_info_t video_parameters[] =
  {
    {
      .name      = "codec",
      .long_name = TRS("Codec"),
      .type      = BG_PARAMETER_MULTI_MENU,
    },
    BG_ENCODER_FRAMERATE_PARAMS,
    { /* */ }
  };

static void create_codec_parameter(bg_parameter_info_t * parameter_info,
                                   const ffmpeg_codec_info_t ** infos,
                                   int num_infos)
  {
  int i;
  parameter_info[0].multi_names_nc =
    calloc(num_infos+1, sizeof(*parameter_info[0].multi_names));
  parameter_info[0].multi_labels_nc =
    calloc(num_infos+1, sizeof(*parameter_info[0].multi_labels));
  parameter_info[0].multi_parameters_nc =
    calloc(num_infos+1, sizeof(*parameter_info[0].multi_parameters));

  for(i = 0; i < num_infos; i++)
    {
    parameter_info[0].multi_names_nc[i]  =
      bg_strdup(parameter_info[0].multi_names_nc[i], infos[i]->name);

    parameter_info[0].multi_labels_nc[i] =
      bg_strdup(parameter_info[0].multi_labels_nc[i], infos[i]->long_name);

    if(infos[i]->parameters)
      parameter_info[0].multi_parameters_nc[i] =
        bg_parameter_info_copy_array(infos[i]->parameters);
    }
  parameter_info[0].val_default.val_str =
    bg_strdup(parameter_info[0].val_default.val_str, infos[0]->name);
  bg_parameter_info_set_const_ptrs(&parameter_info[0]);
  }


bg_parameter_info_t * 
bg_ffmpeg_create_audio_parameters(const ffmpeg_format_info_t * format_info)
  {
  int i, j, num_infos = 0;
  bg_parameter_info_t * ret;
  const ffmpeg_codec_info_t ** infos = NULL;
  
  /* Create codec array */
  i = 0;
  while(format_info[i].name)
    {
    if(!format_info[i].audio_codecs)
      {
      i++;
      continue;
      }
    j = 0;
    while(format_info[i].audio_codecs[j] != CODEC_ID_NONE)
      {
      infos = add_codec_info(infos, format_info[i].audio_codecs[j],
                             &num_infos);
      j++;
      }
    i++;
    }

  if(!infos)
    return NULL;
  
  /* Create parameters */
  ret = bg_parameter_info_copy_array(audio_parameters);
  create_codec_parameter(&ret[0], infos, num_infos);
  free(infos);
  
  return ret;
  }

bg_parameter_info_t * 
bg_ffmpeg_create_video_parameters(const ffmpeg_format_info_t * format_info)
  {
  int i, j, num_infos = 0;
  bg_parameter_info_t * ret;
  const ffmpeg_codec_info_t ** infos = NULL;

  /* Create codec array */
  i = 0;
  while(format_info[i].name)
    {
    if(!format_info[i].video_codecs)
      {
      i++;
      continue;
      }
    j = 0;
    while(format_info[i].video_codecs[j] != CODEC_ID_NONE)
      {
      infos = add_codec_info(infos, format_info[i].video_codecs[j],
                             &num_infos);
      j++;
      }
    i++;
    }

  if(!infos)
    return NULL;
  
  /* Create parameters */

  /* Create parameters */
  ret = bg_parameter_info_copy_array(video_parameters);
  create_codec_parameter(&ret[0], infos, num_infos);
  free(infos);
  
  return ret;
  }

enum CodecID
bg_ffmpeg_find_audio_encoder(const ffmpeg_format_info_t * format, const char * name)
  {
  int i = 0, found = 0;
  enum CodecID ret = CODEC_ID_NONE;
  
  while(audio_codecs[i].name)
    {
    if(!strcmp(name, audio_codecs[i].name))
      {
      ret = audio_codecs[i].id;
      break;
      }
    i++;
    }

  i = 0;
  while(format->audio_codecs[i] != CODEC_ID_NONE)
    {
    if(format->audio_codecs[i] == ret)
      {
      found = 1;
      break;
      }
    i++;
    }

  if(!found)
    {
    bg_log(BG_LOG_ERROR, LOG_DOMAIN,
           "Audio codec %s is not supported by %s",
           name, format->name);
    ret = CODEC_ID_NONE;
    }
  
  return ret;
  }

enum CodecID
bg_ffmpeg_find_video_encoder(const ffmpeg_format_info_t * format, const char * name)
  {
  int i = 0, found = 0;
  enum CodecID ret = CODEC_ID_NONE;
  
  while(video_codecs[i].name)
    {
    if(!strcmp(name, video_codecs[i].name))
      {
      ret = video_codecs[i].id;
      break;
      }
    i++;
    }

  i = 0;
  while(format->video_codecs[i] != CODEC_ID_NONE)
    {
    if(format->video_codecs[i] == ret)
      {
      found = 1;
      break;
      }
    i++;
    }

  if(!found)
    {
    bg_log(BG_LOG_ERROR, LOG_DOMAIN,
           "Video codec %s is not supported by %s",
           name, format->name);
    ret = CODEC_ID_NONE;
    }
  
  return ret;
  }


typedef struct
  {
  const char * s;
  int i;
  } enum_t;

#define PARAM_INT(n, var) \
  if(!strcmp(n, name)) \
    { \
    ctx->var = val->val_i;                         \
    found = 1; \
    }

#define PARAM_INT_SCALE(n, var, scale)   \
  if(!strcmp(n, name)) \
    { \
    ctx->var = val->val_i * scale;                 \
    found = 1; \
    }

#define PARAM_STR_INT_SCALE(n, var, scale) \
  if(!strcmp(n, name)) \
    { \
    ctx->var = atoi(val->val_str) * scale; \
    found = 1; \
    }


#define PARAM_QP2LAMBDA(n, var)   \
  if(!strcmp(n, name)) \
    { \
    ctx->var = (int)(val->val_f * FF_QP2LAMBDA+0.5);  \
    found = 1; \
    }
#define PARAM_FLOAT(n, var)   \
  if(!strcmp(n, name)) \
    { \
    ctx->var = val->val_f;                 \
    found = 1; \
    }

#define PARAM_CMP_CHROMA(n,var)              \
  {                                             \
  if(!strcmp(n, name))                    \
    {                                           \
    if(val->val_i)                            \
      ctx->var |= FF_CMP_CHROMA;                \
    else                                        \
      ctx->var &= ~FF_CMP_CHROMA;               \
                                                \
    found = 1;                                  \
    }                                           \
  }

#define PARAM_FLAG(n,flag)                  \
  {                                             \
  if(!strcmp(n, name))                    \
    {                                           \
    if(val->val_i)                            \
      ctx->flags |= flag;                \
    else                                        \
      ctx->flags &= ~flag;               \
                                                \
    found = 1;                                  \
    }                                           \
  }

#define PARAM_FLAG2(n,flag)                  \
  {                                             \
  if(!strcmp(n, name))                    \
    {                                           \
    if(val->val_i)                            \
      ctx->flags2 |= flag;                \
    else                                        \
      ctx->flags2 &= ~flag;               \
                                                \
    found = 1;                                  \
    }                                           \
  }

  

const enum_t me_method[] =
  {
    { "Zero",  ME_ZERO },
    { "Phods", ME_PHODS },
    { "Log",   ME_LOG },
    { "X1",    ME_X1 },
    { "Epzs",  ME_EPZS },
    { "Full",  ME_FULL }
  };

const enum_t prediction_method[] =
  {
    { "Left",   FF_PRED_LEFT },
    { "Plane",  FF_PRED_PLANE },
    { "Median", FF_PRED_MEDIAN }
  };

const enum_t compare_func[] =
  {
    { "SAD",  FF_CMP_SAD },
    { "SSE",  FF_CMP_SSE },
    { "SATD", FF_CMP_SATD },
    { "DCT",  FF_CMP_DCT },
    { "PSNR", FF_CMP_PSNR },
    { "BIT",  FF_CMP_BIT },
    { "RD",   FF_CMP_RD },
    { "ZERO", FF_CMP_ZERO },
    { "VSAD", FF_CMP_VSAD },
    { "VSSE", FF_CMP_VSSE },
    { "NSSE", FF_CMP_NSSE }
  };

const enum_t mb_decision[] =
  {
    { "Use compare function", FF_MB_DECISION_SIMPLE },
    { "Fewest bits",          FF_MB_DECISION_BITS },
    { "Rate distoration",     FF_MB_DECISION_RD }
  };

#define PARAM_ENUM(n, var, arr) \
  if(!strcmp(n, name)) \
    { \
    for(i = 0; i < sizeof(arr)/sizeof(arr[0]); i++) \
      {                                             \
      if(!strcmp(val->val_str, arr[i].s))       \
        {                                           \
        ctx->var = arr[i].i;                        \
        break;                                      \
        }                                           \
      }                                             \
    found = 1;                                      \
    }

#if LIBAVCODEC_VERSION_MAJOR >= 54
#define PARAM_DICT_STRING(n, ffmpeg_key) \
  if(!strcmp(n, name)) \
    { \
    if(val->val_str && (val->val_str[0] != '$')) \
      av_dict_set(options,                      \
                  ffmpeg_key, val->val_str, 0); \
    }

#define PARAM_DICT_FLOAT(n, ffmpeg_key) \
  if(!strcmp(n, name)) \
    { \
    char * str; \
    str = bg_sprintf("%f", val->val_f); \
    av_dict_set(options, ffmpeg_key, str, 0); \
    free(str); \
    }

#define PARAM_DICT_INT(n, ffmpeg_key) \
  if(!strcmp(n, name)) \
    { \
    char * str; \
    str = bg_sprintf("%d", val->val_i); \
    av_dict_set(options, ffmpeg_key, str, 0); \
    free(str); \
    }
#endif


void
bg_ffmpeg_set_codec_parameter(AVCodecContext * ctx,
#if LIBAVCODEC_VERSION_MAJOR >= 54
                              AVDictionary ** options,
#endif

                              const char * name,
                              const bg_parameter_value_t * val)
  {
  int found = 0, i;
/*
 *   IMPORTANT: To keep the mess at a reasonable level,
 *   *all* parameters *must* appear in the same order as in
 *   the AVCocecContext structure, except the flags, which come at the very end
 */
  
  PARAM_INT_SCALE("ff_bit_rate_video",bit_rate,1000);
  
  PARAM_STR_INT_SCALE("ff_bit_rate_str", bit_rate, 1000);

  PARAM_INT_SCALE("ff_bit_rate_tolerance",bit_rate_tolerance,1000);
  PARAM_ENUM("ff_me_method",me_method,me_method);
  PARAM_INT("ff_gop_size",gop_size);
  PARAM_FLOAT("ff_qcompress",qcompress);
  PARAM_FLOAT("ff_qblur",qblur);
  PARAM_INT("ff_qmin",qmin);
  PARAM_INT("ff_qmax",qmax);
  PARAM_INT("ff_max_qdiff",max_qdiff);
  PARAM_INT("ff_max_b_frames",max_b_frames);
  PARAM_FLOAT("ff_b_quant_factor",b_quant_factor);
  PARAM_INT("ff_b_frame_strategy",b_frame_strategy);
  PARAM_INT("ff_luma_elim_threshold",luma_elim_threshold);
  PARAM_INT("ff_chroma_elim_threshold",chroma_elim_threshold);
  PARAM_INT("ff_strict_std_compliance",strict_std_compliance);
  PARAM_QP2LAMBDA("ff_b_quant_offset",b_quant_offset);
  PARAM_INT("ff_rc_min_rate",rc_min_rate);
  PARAM_INT("ff_rc_max_rate",rc_max_rate);
  PARAM_INT_SCALE("ff_rc_buffer_size",rc_buffer_size,1000);
  PARAM_FLOAT("ff_rc_buffer_aggressivity",rc_buffer_aggressivity);
  PARAM_FLOAT("ff_i_quant_factor",i_quant_factor);
  PARAM_QP2LAMBDA("ff_i_quant_offset",i_quant_offset);
  PARAM_FLOAT("ff_rc_initial_cplx",rc_initial_cplx);
  PARAM_FLOAT("ff_lumi_masking",lumi_masking);
  PARAM_FLOAT("ff_temporal_cplx_masking",temporal_cplx_masking);
  PARAM_FLOAT("ff_spatial_cplx_masking",spatial_cplx_masking);
  PARAM_FLOAT("ff_p_masking",p_masking);
  PARAM_FLOAT("ff_dark_masking",dark_masking);
  PARAM_ENUM("ff_prediction_method",prediction_method,prediction_method);
  PARAM_ENUM("ff_me_cmp",me_cmp,compare_func);
  PARAM_CMP_CHROMA("ff_me_cmp_chroma",me_cmp);
  PARAM_ENUM("ff_me_sub_cmp",me_sub_cmp,compare_func);
  PARAM_CMP_CHROMA("ff_me_sub_cmp_chroma",me_sub_cmp);
  PARAM_ENUM("ff_mb_cmp",mb_cmp,compare_func);
  PARAM_CMP_CHROMA("ff_mb_cmp_chroma",mb_cmp);
  PARAM_ENUM("ff_ildct_cmp",ildct_cmp,compare_func);
  PARAM_CMP_CHROMA("ff_ildct_cmp_chroma",ildct_cmp);
  PARAM_INT("ff_dia_size",dia_size);
  PARAM_INT("ff_last_predictor_count",last_predictor_count);
  PARAM_INT("ff_pre_me",pre_me);
  PARAM_ENUM("ff_me_pre_cmp",me_pre_cmp,compare_func);
  PARAM_CMP_CHROMA("ff_pre_me_cmp_chroma",me_pre_cmp);
  PARAM_INT("ff_pre_dia_size",pre_dia_size);
  PARAM_INT("ff_me_subpel_quality",me_subpel_quality);
  PARAM_INT("ff_me_range",me_range);
  PARAM_ENUM("ff_mb_decision",mb_decision,mb_decision);
  PARAM_INT("ff_scenechange_threshold",scenechange_threshold);
  PARAM_QP2LAMBDA("ff_lmin", lmin);
  PARAM_QP2LAMBDA("ff_lmax", lmax);
  PARAM_INT("ff_noise_reduction",noise_reduction);
  PARAM_INT_SCALE("ff_rc_initial_buffer_occupancy",rc_initial_buffer_occupancy,1000);
  PARAM_INT("ff_inter_threshold",inter_threshold);
  PARAM_INT("ff_quantizer_noise_shaping",quantizer_noise_shaping);
  PARAM_INT("ff_me_threshold",me_threshold);
  PARAM_INT("ff_mb_threshold",mb_threshold);
  PARAM_INT("ff_nsse_weight",nsse_weight);
  PARAM_FLOAT("ff_border_masking",border_masking);
  PARAM_QP2LAMBDA("ff_mb_lmin", mb_lmin);
  PARAM_QP2LAMBDA("ff_mb_lmax", mb_lmax);
  PARAM_INT("ff_me_penalty_compensation",me_penalty_compensation);
  PARAM_INT("ff_bidir_refine",bidir_refine);
  PARAM_INT("ff_brd_scale",brd_scale);
  PARAM_INT("ff_scenechange_factor",scenechange_factor);
  PARAM_FLAG("ff_flag_qscale",CODEC_FLAG_QSCALE);
  PARAM_FLAG("ff_flag_4mv",CODEC_FLAG_4MV);
  PARAM_FLAG("ff_flag_qpel",CODEC_FLAG_QPEL);
  PARAM_FLAG("ff_flag_gmc",CODEC_FLAG_GMC);
  PARAM_FLAG("ff_flag_mv0",CODEC_FLAG_MV0);
  //  PARAM_FLAG("ff_flag_part",CODEC_FLAG_PART);
  PARAM_FLAG("ff_flag_gray",CODEC_FLAG_GRAY);
  PARAM_FLAG("ff_flag_emu_edge",CODEC_FLAG_EMU_EDGE);
  PARAM_FLAG("ff_flag_normalize_aqp",CODEC_FLAG_NORMALIZE_AQP);
  //  PARAM_FLAG("ff_flag_alt_scan",CODEC_FLAG_ALT_SCAN);
#if LIBAVCODEC_VERSION_INT < ((52<<16)+(0<<8)+0)
  PARAM_FLAG("ff_flag_trellis_quant",CODEC_FLAG_TRELLIS_QUANT);
#else
  PARAM_INT("ff_trellis",trellis);
#endif
  PARAM_FLAG("ff_flag_bitexact",CODEC_FLAG_BITEXACT);
  PARAM_FLAG("ff_flag_ac_pred",CODEC_FLAG_AC_PRED);
  //  PARAM_FLAG("ff_flag_h263p_umv",CODEC_FLAG_H263P_UMV);
  PARAM_FLAG("ff_flag_cbp_rd",CODEC_FLAG_CBP_RD);
  PARAM_FLAG("ff_flag_qp_rd",CODEC_FLAG_QP_RD);
  //  PARAM_FLAG("ff_flag_h263p_aiv",CODEC_FLAG_H263P_AIV);
  //  PARAM_FLAG("ffx_flag_obmc",CODEC_FLAG_OBMC);
  PARAM_FLAG("ff_flag_loop_filter",CODEC_FLAG_LOOP_FILTER);
  //  PARAM_FLAG("ff_flag_h263p_slice_struct",CODEC_FLAG_H263P_SLICE_STRUCT);
  PARAM_FLAG("ff_flag_closed_gop",CODEC_FLAG_CLOSED_GOP);
  PARAM_FLAG2("ff_flag2_fast",CODEC_FLAG2_FAST);
  PARAM_FLAG2("ff_flag2_strict_gop",CODEC_FLAG2_STRICT_GOP);

#if LIBAVCODEC_VERSION_MAJOR >= 54
  PARAM_DICT_STRING("libx264_preset", "preset");
  PARAM_DICT_STRING("libx264_tune",   "tune");
  PARAM_DICT_FLOAT("libx264_crf", "crf");
  PARAM_DICT_FLOAT("libx264_qp", "qp");
#endif
  
  }

enum PixelFormat * bg_ffmpeg_get_pixelformats(enum CodecID id)
  {
  int i = 0;
  while(video_codecs[i].name)
    {
    if(video_codecs[i].id == id)
      return video_codecs[i].pixelformats;
    i++;
    }
  return NULL;
  }
