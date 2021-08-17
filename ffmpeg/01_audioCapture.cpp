/*
 * @Descripttion: 利用FFmpeg采集音频数据
 * @version: 
 * @Author: machun Michael
 * @Date: 2021-08-09 06:11:59
 * @LastEditors: machun Michael
 * @LastEditTime: 2021-08-17 02:18:14
 */

#include <stdio.h>
#include <stdlib.h>

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

    // 0. 注册音频设备
    avdevice_register_all();

    // 指定用alsa库来采集音频
    ptInputFmt = av_find_input_format("alsa");
    if (NULL == ptInputFmt)
    {
        printf("av_find_input_format failed\n");
        return -1;
    }

    // 1. 打开音频设备
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

    //create file
    FILE *pOutfile = fopen(achOutPath, "w+");
    if (NULL == pOutfile)
    {
        av_log(NULL, AV_LOG_DEBUG, "fopen failed\n");
        return -1;
    }
    
    ptSwrCtx = swr_alloc_set_opts(NULL,                //ctx
                                 AV_CH_LAYOUT_STEREO, //输出channel布局
                                 AV_SAMPLE_FMT_S16,   //输出的采样格式
                                 44100,               //采样率
                                 AV_CH_LAYOUT_STEREO, //输入channel布局
                                 AV_SAMPLE_FMT_FLT,   //输入的采样格式
                                 44100,               //输入的采样率
                                 0, NULL);
    
    swr_init(ptSwrCtx);
    //4096/4=1024/2=512
    //创建输入缓冲区
    av_samples_alloc_array_and_samples(&pbySrcAudioData,   //输入缓冲区地址
                                       &nSrcLineSize,     //缓冲区的大小
                                       2,                 //通道个数
                                       512,               //单通道采样个数
                                       AV_SAMPLE_FMT_FLT, //采样格式
                                       0);
    
    //创建输出缓冲区
    av_samples_alloc_array_and_samples(&pbyDstAudioData,   //输出缓冲区地址
                                       &nDstLineSize,     //缓冲区的大小
                                       2,                 //通道个数
                                       512,               //单通道采样个数
                                       AV_SAMPLE_FMT_S16, //采样格式
                                       0);

    // 2. 读取音频数据
    av_init_packet(&tAvPacket);  // packet 初始化

    // av_read_frame返回0表示成功
    int nCount = 0;
    while(0 == (nRet = av_read_frame(ptFmtCtx, &tAvPacket)) && nCount++ < 100)
    {
        av_log(NULL, AV_LOG_DEBUG, "av_read_frame succ, [%d] size:%d\n", nCount, tAvPacket.size);

        memcpy(pbySrcAudioData[0], tAvPacket.data, tAvPacket.size);

         //重采样
        swr_convert(ptSwrCtx,                           //重采样的上下文
                    pbyDstAudioData,                    //输出结果缓冲区
                    512,                                //每个通道的采样数
                    (const uint8_t **)pbySrcAudioData,  //输入缓冲区
                    512);                               //输入单个通道的采样数

         //write file
        //fwrite(pkt.data, 1, pkt.size, outfile);
        fwrite(pbyDstAudioData[0], 1, nDstLineSize, pOutfile);
        fflush(pOutfile);        
        av_packet_unref(&tAvPacket); // packet 反初始化
    }

    fclose(pOutfile);

    //释放输入输出缓冲区
    if(pbySrcAudioData)
    {
        av_freep(&pbySrcAudioData[0]);
    }
    av_freep(&pbySrcAudioData);

    if(pbyDstAudioData)
    {
        av_freep(&pbyDstAudioData[0]);
    }
    av_freep(&pbyDstAudioData);

    //释放重采样的上下文
    swr_free(&ptSwrCtx);

    // 打开后需要关闭
    avformat_close_input(&ptFmtCtx);

    av_log(NULL, AV_LOG_DEBUG, "audio capture finish!\n");

    return 0;
}