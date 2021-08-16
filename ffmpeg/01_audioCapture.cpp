/*
 * @Descripttion: 利用FFmpeg采集音频数据
 * @version: 
 * @Author: machun Michael
 * @Date: 2021-08-09 06:11:59
 * @LastEditors: machun Michael
 * @LastEditTime: 2021-08-16 05:41:54
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
}

int main()
{
    int nRet = 0;
    // char achDevName[64] = "hw:0,0";
    char achErrBuf[512] = {0};
    const AVInputFormat *ptInputFmt = NULL;
    AVFormatContext *ptFmtCtx = NULL;
    AVDictionary *ptOpt = NULL;
    AVPacket tAvPacket;
    char achOutPath[128] = "/home/machun/ffmpeg_learn/audioCapture.pcm";

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
        printf("avformat_open_input failed, error:%s\n", achErrBuf);
        return -1;
    }
    else
    {
        printf("avformat_open_input succ\n");
    }

    //create file
    FILE *pOutfile = fopen(ouachOutPatht, "w+");
    
    SwrContext* swr_ctx = init_swr();
    
    //4096/4=1024/2=512
    //创建输入缓冲区
    av_samples_alloc_array_and_samples(&src_data,         //输出缓冲区地址
                                       &src_linesize,     //缓冲区的大小
                                       2,                 //通道个数
                                       512,               //单通道采样个数
                                       AV_SAMPLE_FMT_FLT, //采样格式
                                       0);
    
    //创建输出缓冲区
    av_samples_alloc_array_and_samples(&dst_data,         //输出缓冲区地址
                                       &dst_linesize,     //缓冲区的大小
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
        printf("av_read_frame succ, [%d] size:%d\n", nCount, tAvPacket.size);
        av_packet_unref(&tAvPacket); // packet 反初始化
    }

    // 打开后需要关闭
    avformat_close_input(&ptFmtCtx);

    printf("audio capture finish!\n");

    return 0;
}