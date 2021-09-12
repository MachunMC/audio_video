/*
 * @Descripttion: 创建AAC编码器
 * @version: 
 * @Author: machun Michael
 * @Date: 2021-08-18 16:45:34
 * @LastEditors: machun Michael
 * @LastEditTime: 2021-08-25 16:44:25
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define __STDC_CONSTANT_MACROS 
extern "C"
{
    #include "libavutil/log.h"
    #include "libavutil/avutil.h"
    #include "libavutil/channel_layout.h"
    #include "libavcodec/avcodec.h"
    #include "libswresample/swresample.h"
    #include "libavdevice/avdevice.h"
    #include "libavcodec/packet.h"
}

AVFormatContext* OpenAudioDev(void)
{
    int nRet = 0;
    const AVInputFormat *ptInputFormat = NULL;
    AVFormatContext *ptFormatCtx = NULL;
    AVDictionary *ptOpt = NULL;
    char achErrBuf[1024+1] = {0};

    // 注册音频设备
    avdevice_register_all();

    // 指定使用alsa库采集音频
    ptInputFormat = av_find_input_format("alsa");
    if (NULL == ptInputFormat)
    {
        av_log(NULL, AV_LOG_ERROR, "av_find_input_format failed\n");
        return NULL;
    }

    // 打开音频设备
    nRet = avformat_open_input(&ptFormatCtx, "hw:0,0", ptInputFormat, &ptOpt);
    if (0 != nRet)
    {
        av_strerror(nRet, achErrBuf, sizeof(achErrBuf));
        av_log(NULL, AV_LOG_ERROR, "avformat_open_input failed, ret:%d, %s\n", nRet, achErrBuf);
        return NULL;
    }
    else
    {
        av_log(NULL, AV_LOG_DEBUG, "avformat_open_input succ\n");
        return ptFormatCtx;
    }
}

void CloseAudioDev(AVFormatContext **ptFormatCtx)
{
    avformat_close_input(ptFormatCtx);
}

AVCodecContext* OpenAACEncoder(void)
{
    int nRet = 0;
    const AVCodec *ptAvCodec = NULL;
    AVCodecContext *ptCodecCtx = NULL;
    AVDictionary *ptAvOptions = NULL;
    
    // 创建编码器(查找编码器)
    // ptAvCodec = avcodec_find_encoder(AV_CODEC_ID_AAC);
    ptAvCodec = avcodec_find_encoder_by_name("libfdk_aac");
    if (NULL == ptAvCodec)
    {
        av_log(NULL, AV_LOG_ERROR, "avcodec_find_encoder failed\n");
        return NULL;
    }

    // 创建上下文
    ptCodecCtx = avcodec_alloc_context3(ptAvCodec);
    if (NULL == ptCodecCtx)
    {
        av_log(NULL, AV_LOG_ERROR, "avcodec_alloc_context3 failed\n");
        return NULL;
    }

    // 设置音频三要素：采样率、采样大小、声道数
    ptCodecCtx->sample_rate = 44100;                   // 采样率44100
    ptCodecCtx->sample_fmt = AV_SAMPLE_FMT_S16;        // 采样大小为16位
    ptCodecCtx->channel_layout = AV_CH_LAYOUT_STEREO;  // 通道布局，立体声
    ptCodecCtx->channels = 2;                          // 通道个数
    ptCodecCtx->bit_rate = 0;                          // 输出码率，设置了profile，则需要将码率设置为0才能生效
    ptCodecCtx->profile = FF_PROFILE_AAC_HE_V2;        // 编码器规格，AAC_HE_V2

    // 打开编码器
    nRet = avcodec_open2(ptCodecCtx, ptAvCodec, &ptAvOptions);
    if (0 != nRet)
    {
        av_log(NULL, AV_LOG_ERROR, "avcodec_open2 failed, ret:%d\n", nRet);
        return NULL;
    }
    else
    {
        av_log(NULL, AV_LOG_DEBUG, "avcodec_open2 succ\n");
        return ptCodecCtx;
    }
}

int AllocInOutBuffer(AVFrame **ptFrame, AVPacket **ptPacket)
{
    int nRet = 0;
    AVFrame *ptAvFrame = NULL;

    if (NULL == ptFrame || NULL == ptPacket)
    {
        av_log(NULL, AV_LOG_ERROR, "input param invalid\n");
        return -1;
    }
    
    // 为音频输入数据分配空间
    ptAvFrame = av_frame_alloc();
    if (ptAvFrame == NULL)
    {
        av_log(NULL, AV_LOG_ERROR, "av_frame_alloc failed\n");
        return -1;
    }
    *ptFrame = ptAvFrame;

    ptAvFrame->nb_samples     = 512;                      // 单通道一个音频桢的采样数
    ptAvFrame->format         = AV_SAMPLE_FMT_S16;    // 采样大小
    ptAvFrame->channel_layout = AV_CH_LAYOUT_STEREO;  // 声道数
    
    // int av_frame_get_buffer(AVFrame *frame, int align);
    nRet = av_frame_get_buffer(ptAvFrame, 0);
    if (0 != nRet)
    {
        av_log(NULL, AV_LOG_ERROR, "av_frame_get_buffer failed\n");
        return -1;
    }   

    // 为音频输出数据分配空间
    *ptPacket = av_packet_alloc();
    if (NULL == *ptPacket)
    {
        av_log(NULL, AV_LOG_ERROR, "av_packet_alloc failed\n");
        return -1;
    }
    else
    {
        return 0;
    }
}

int SetSwrParam(SwrContext **ptSwrCxt, uint8_t ***pbyInAudioData, int *pnInLineSize, uint8_t ***pbyOutAudioData, int *pnOutLineSize)
{
    int nRet = 0;
    char achErrBuf[1024+1] = {0};

    if (NULL == ptSwrCxt || NULL == pbyInAudioData || NULL == pnInLineSize || NULL == pbyOutAudioData || NULL == pnOutLineSize)
    {
        av_log(NULL, AV_LOG_ERROR, "input param invalid\n");
        return -1;
    }
    
    // 创建重采样上下文，并设置参数
    *ptSwrCxt = swr_alloc_set_opts(NULL,                 // 上下文
                                  AV_CH_LAYOUT_STEREO,  // [输出] 通道布局，立体声
                                  AV_SAMPLE_FMT_S16,    // [输出] 采样格式，采样大小 16位
                                  512,                  // [输出] 采样率
                                  AV_CH_LAYOUT_STEREO,  // [输入] 通道布局，立体声
                                  AV_SAMPLE_FMT_FLT,    // [输入] 采样格式
                                  512,                  // [输入] 采样率
                                  0, NULL);
    if (NULL == *ptSwrCxt)
    {
        av_log(NULL, AV_LOG_ERROR, "swr_alloc_set_opts failed\n");
        return -1;
    }

    // 上下文初始化
    nRet = swr_init(*ptSwrCxt);
    if (0 != nRet)
    {
        av_strerror(nRet, achErrBuf, sizeof(achErrBuf));
        av_log(NULL, AV_LOG_ERROR, "swr_init failed, ret:%d, error:%s\n", nRet, achErrBuf);
        return -1;
    }

    // 创建输入缓冲区
    // int av_samples_alloc_array_and_samples(uint8_t ***audio_data, int *linesize, int nb_channels,
                                    //    int nb_samples, enum AVSampleFormat sample_fmt, int align);
    nRet = av_samples_alloc_array_and_samples(pbyInAudioData,   // 输入缓冲区地址
                                              pnInLineSize,      // 输入缓冲区大小
                                              2,                 // 输出通道数
                                              512,               // 单通道采样个数
                                              AV_SAMPLE_FMT_FLT, // 采样格式
                                              0);
    if (nRet < 0)
    {
        av_strerror(nRet, achErrBuf, sizeof(achErrBuf));
        av_log(NULL, AV_LOG_ERROR, "av_samples_alloc_array_and_samples failed, ret:%d, error:%s\n", nRet, achErrBuf);
        return -1;
    }

    // 创建输出缓冲区
    nRet = av_samples_alloc_array_and_samples(pbyOutAudioData,   // 输出缓冲区地址
                                              pnOutLineSize,      // 输出缓冲区大小
                                              2,                  // 输出通道数
                                              512,                // 单通道采样个数
                                              AV_SAMPLE_FMT_S16,  // 采样格式
                                              0);
    if (nRet < 0)
    {
        av_strerror(nRet, achErrBuf, sizeof(achErrBuf));
        av_log(NULL, AV_LOG_ERROR, "av_samples_alloc_array_and_samples failed, ret:%d, error:%s\n", nRet, achErrBuf);
        return -1;
    }

    return 0;
}

int ReadAudioData()
{
    FILE *pFile = NULL;
    AVPacket tFramPacket;
    char achPath[256+1] = "/home/machun/ffmpeg_learn/aac_encoder.aac";

    memset(&tFramPacket, 0, sizeof(tFramPacket));

    av_init_packet(&tFramPacket);

    pFile = fopen(achPath, "w+");
    if (NULL == pFile)
    {
        av_log(NULL, AV_LOG_ERROR, "fopen %s failed\n", achPath);
        return -1;
    }
    
    // 读取音频数据
    while(0 == av_read_frame(ptFormatCtx, &tFramPacket))
    {
        av_log(NULL, AV_LOG_DEBUG, "av_read_frame succ, size:%d\n", tFramPacket.size);

        // 重采样
        swr_convert(ptSwrCxt,                           // 重采样上下文
                    pbyOutAudioData,                    // 输出数据
                    512,
                    (const uint8_t **)pbyInAudioData,  // 输入缓冲区
                    512);
        
        fwrite(pbyOutAudioData[0], 1, nOutLineSize, pFile);

        av_packet_unref(&tFramPacket);
    }

    fclose(pFile);    

    return 0;
}

int main()
{
    int nRet = 0;
    int nInLineSize = 0;
    int nOutLineSize = 0;
    uint8_t **pbyInAudioData = NULL;
    uint8_t **pbyOutAudioData = NULL;
    SwrContext *ptSwrCxt = NULL;
    AVFrame *ptFrame = NULL;
    AVPacket *ptPacket = NULL;
    AVCodecContext *ptCodecCtx = NULL;    // AAC编码器的上下文
    AVFormatContext *ptFormatCtx = NULL;  // 音频设备的上下文
    
    av_log_set_level(AV_LOG_DEBUG);

    // 打开音频设备，创建上下文
    ptFormatCtx = OpenAudioDev();
    if (NULL == ptFormatCtx)
    {
        av_log(NULL, AV_LOG_ERROR, "open audio device failed\n");
        return -1;
    }

    // 创建aac编码器
    ptCodecCtx = OpenAACEncoder();
    if (NULL == ptCodecCtx)
    {
        av_log(NULL, AV_LOG_ERROR, "open aac encoder failed\n");
        return -1;
    }

    // 为输入输出数据分配空间
    nRet = AllocInOutBuffer(&ptFrame, &ptPacket);
    if (0 != nRet)
    {
        av_log(NULL, AV_LOG_ERROR, "alloc in and out buffer failed, ret:%d\n", nRet);
        return -1;
    }

    // 设置重采样参数
    nRet = SetSwrParam(&ptSwrCxt, &pbyInAudioData, &nInLineSize, &pbyOutAudioData, &nOutLineSize);
    if (0 != nRet)
    {
        av_log(NULL, AV_LOG_ERROR, "SetSwrParam failed, ret:%d\n", nRet);
        return -1;
    }

    // 读取音频数据，并编码保存到文件
    nRet = ReadAudioData();
    if (0 != nRet)
    {
        av_log(NULL, AV_LOG_ERROR, "ReadAudioData failed, ret:%d\n", nRet);
        return -1;
    }

    // 释放缓冲区资源
    if (NULL != pbyInAudioData)
    {
        av_freep(pbyInAudioData[0]);
    }

    if (NULL != pbyOutAudioData)
    {
        av_freep(pbyOutAudioData[0]);
    }

    // 关闭重采样的上下文
    swr_free(&ptSwrCxt);

    // 关闭设备
    CloseAudioDev(&ptFormatCtx);

    av_log(NULL, AV_LOG_DEBUG, "hello ffmpeg\n");
    
    return 0;
}