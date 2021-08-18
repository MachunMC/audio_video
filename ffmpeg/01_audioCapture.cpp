/*
 * @Descripttion: 利用FFmpeg采集音频数据
 * @version: 
 * @Author: machun Michael
 * @Date: 2021-08-09 06:11:59
 * @LastEditors: machun Michael
 * @LastEditTime: 2021-08-18 16:45:00
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define __STDC_CONSTANT_MACROS 
extern "C"
{
    #include "libavdevice/avdevice.h"
    #include "libavformat/avformat.h"
    #include "libavutil/log.h"
    #include "libavcodec/avcodec.h"
    #include "libavcodec/packet.h"
    #include "libswresample/swresample.h"
}

int main()
{
    int nRet = 0;    
    int nCount = 0;
    int nSrcLineSize = 0;
    int nDstLineSize = 0;
    uint8_t **pbySrcAudioData = NULL;
    uint8_t **pbyDstAudioData = NULL;
    char achErrBuf[512] = {0};
    const AVInputFormat *ptInputFmt = NULL;
    AVFormatContext *ptFmtCtx = NULL;
    AVDictionary *ptOpt = NULL;
    SwrContext* ptSwrCtx = NULL;
    AVPacket tAvPacket;
    char achOutPath[128] = "/home/machun/ffmpeg_learn/audioCapture.pcm";

    // 设置打印等级
    av_log_set_level(AV_LOG_DEBUG);

    // 注册音频设备
    avdevice_register_all();

    // 指定用alsa库来采集音频
    ptInputFmt = av_find_input_format("alsa");
    if (NULL == ptInputFmt)
    {
        printf("av_find_input_format failed\n");
        return -1;
    }

    // 打开音频设备
    nRet = avformat_open_input(&ptFmtCtx, "hw:0,0", ptInputFmt, &ptOpt);
    if (nRet < 0)
    {
        av_strerror(nRet, achErrBuf, sizeof(achErrBuf));
        av_log(NULL, AV_LOG_DEBUG, "avformat_open_input failed, error:%s\n", achErrBuf);
        return -1;
    }
    else
    {
        av_log(NULL, AV_LOG_DEBUG, "avformat_open_input succ\n");
    }

    FILE *pOutfile = fopen(achOutPath, "w+");
    if (NULL == pOutfile)
    {
        av_log(NULL, AV_LOG_DEBUG, "fopen failed\n");
        return -1;
    }
    
    // 创建重采样上下文，设置参数
    // struct SwrContext *swr_alloc_set_opts(struct SwrContext *s,
    //                                   int64_t out_ch_layout, enum AVSampleFormat out_sample_fmt, int out_sample_rate,
    //                                   int64_t  in_ch_layout, enum AVSampleFormat  in_sample_fmt, int  in_sample_rate,
    //                                   int log_offset, void *log_ctx);
    ptSwrCtx = swr_alloc_set_opts(NULL,               //上下文
                                 AV_CH_LAYOUT_STEREO, //输出channel布局
                                 AV_SAMPLE_FMT_S16,   //输出采样格式
                                 44100,               //输出采样率
                                 AV_CH_LAYOUT_STEREO, //输入channel布局
                                 AV_SAMPLE_FMT_FLT,   //输入采样格式
                                 44100,               //输入采样率
                                 0, NULL);
    if (NULL == ptSwrCtx)
    {
        av_log(NULL, AV_LOG_DEBUG, "swr_alloc_set_opts failed\n");
        return -1;
    }
    
    // 上下文初始化
    nRet = swr_init(ptSwrCtx);
    if (nRet < 0)
    {
        av_log(NULL, AV_LOG_DEBUG, "swr_init failed\n");
        return -1;
    }

    // 创建输入缓冲区
    // int av_samples_alloc_array_and_samples(uint8_t ***audio_data, int *linesize, int nb_channels,
    //                                        int nb_samples, enum AVSampleFormat sample_fmt, int align);
    av_samples_alloc_array_and_samples(&pbySrcAudioData,  // 输入缓冲区地址
                                       &nSrcLineSize,     // 输入缓冲区的大小
                                       2,                 // 通道数
                                       512,               // 单通道采样个数
                                       AV_SAMPLE_FMT_FLT, // 采样格式
                                       0);
    
    // 创建输出缓冲区
    av_samples_alloc_array_and_samples(&pbyDstAudioData,   // 输出缓冲区地址
                                       &nDstLineSize,      // 输出缓冲区大小
                                       2,                  // 通道数
                                       512,                // 单通道采样个数
                                       AV_SAMPLE_FMT_S16,  // 采样格式
                                       0);


    av_init_packet(&tAvPacket);  // packet 初始化

    // 读取音频数据
    while(0 == (nRet = av_read_frame(ptFmtCtx, &tAvPacket)) && nCount++ < 500)
    {
        av_log(NULL, AV_LOG_DEBUG, "av_read_frame succ, [%d] size:%d\n", nCount, tAvPacket.size);

        memcpy(pbySrcAudioData[0], tAvPacket.data, tAvPacket.size);

        // 重采样
        // int swr_convert(struct SwrContext *s, uint8_t **out, int out_count,
        //                 const uint8_t **in , int in_count);
        swr_convert(ptSwrCtx,                           // 重采样的上下文
                    pbyDstAudioData,                    // 输出结果缓冲区
                    512,                                // 每个通道的采样数
                    (const uint8_t **)pbySrcAudioData,  // 输入缓冲区
                    512);                               // 输入单个通道的采样数

        fwrite(pbyDstAudioData[0], 1, nDstLineSize, pOutfile);
        fflush(pOutfile);
        
        av_packet_unref(&tAvPacket); // packet 反初始化
    }

    fclose(pOutfile);

    // 释放输入缓冲区
    if(pbySrcAudioData)
    {
        av_freep(&pbySrcAudioData[0]);
    }

    // 释放输出缓冲区
    if(pbyDstAudioData)
    {
        av_freep(&pbyDstAudioData[0]);
    }

    // 释放重采样的上下文
    swr_free(&ptSwrCtx);

    // 打开后需要关闭
    avformat_close_input(&ptFmtCtx);

    av_log(NULL, AV_LOG_DEBUG, "audio capture finish!\n");

    return 0;
}