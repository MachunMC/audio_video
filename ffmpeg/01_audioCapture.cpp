/*
 * @Descripttion: 利用FFmpeg采集音频数据
 * @version: 
 * @Author: machun Michael
 * @Date: 2021-08-09 06:11:59
 * @LastEditors: machun Michael
 * @LastEditTime: 2021-08-09 07:03:55
 */

#include <stdio.h>
#include <stdlib.h>

#define __STDC_CONSTANT_MACROS 
extern "C"
{
    #include "libavdevice/avdevice.h"
    #include "libavformat/avformat.h"
    #include "libavutil/log.h"
}

int main()
{
    int nRet = 0;
    // char achDevName[64] = "hw:0,0";
    char achErrBuf[512] = {0};
    const AVInputFormat *ptInputFmt = NULL;
    AVFormatContext *ptFmtCtx = NULL;
    AVDictionary *ptOpt = NULL;

    // 0. 注册音频设备
    avdevice_register_all();

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

    // 打开后需要关闭
    avformat_close_input(&ptFmtCtx);

    printf("hello ffmpeg\n");

    return 0;
}