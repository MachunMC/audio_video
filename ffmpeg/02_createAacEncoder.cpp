/*
 * @Descripttion: 创建AAC编码器
 * @version: 
 * @Author: machun Michael
 * @Date: 2021-08-18 16:45:34
 * @LastEditors: machun Michael
 * @LastEditTime: 2021-08-18 16:55:40
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define __STDC_CONSTANT_MACROS 
extern "C"
{
    #include "libavutil/log.h"
    #include "libavutil/avutil.h"
    #include "libavcodec/avcodec.h"
}

int main()
{
    const AVCodec *ptAvCodec = NULL;
    av_log_set_level(AV_LOG_DEBUG);

    av_log(NULL, AV_LOG_DEBUG, "hello ffmpeg\n");

    // 创建编码器
    // const AVCodec *avcodec_find_encoder(enum AVCodecID id);
    // const AVCodec *avcodec_find_encoder_by_name(const char *name);
    ptAvCodec = avcodec_find_encoder(AV_CODEC_ID_AAC);
    if (NULL == ptAvCodec)
    {
        av_log(NULL, AV_LOG_ERROR, "avcodec_find_encoder failed\n");
        return -1;
    }

    // 创建上下文
    // AVCodecContext *avcodec_alloc_context3(const AVCodec *codec);
    avcodec_alloc_context3();

    // 打开编码器
    // int avcodec_open2(AVCodecContext *avctx, const AVCodec *codec, AVDictionary **options);
    avcodec_open2();
    
    return 0;
}