/*
 * @Descripttion: 获取多媒体文件的元数据（meta data）
 * @version: 
 * @Author: machun Michael
 * @Date: 2021-09-09 09:57:25
 * @LastEditors: machun Michael
 * @LastEditTime: 2021-09-09 11:06:31
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define __STDC_CONSTANT_MACROS 
extern "C"
{
    #include "libavutil/log.h"
    #include "libavutil/error.h"
    #include "libavformat/avformat.h"
}

int main()
{
    int nRet = 0;
    AVFormatContext *ptFmtCtx = NULL;
    char achErrBuf[256] = {0};
    char achUrl[256] = "/home/machun/ffmpeg_learn/vs_space/wanfeng.mp4";

    // 打开输入流
    nRet = avformat_open_input(&ptFmtCtx, achUrl, NULL, NULL);
    if (0 != nRet)
    {
        av_strerror(nRet, achErrBuf, sizeof(achErrBuf));
        av_log(NULL, AV_LOG_ERROR, "avformat_open_input failed, ret:%d, error:%s\n", nRet, achErrBuf);
        return -1;
    }

    // 获取输入流详细信息
    av_dump_format(ptFmtCtx, 0, achUrl, 0);


    // 关闭输入流，释放所有上下文，并将ptFmtCtx设置为NULL
    avformat_close_input(&ptFmtCtx);

    return 0;
}