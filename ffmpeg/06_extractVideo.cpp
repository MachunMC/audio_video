/*
 * @Descripttion: 
 * @version: 
 * @Author: machun Michael
 * @Date: 2021-09-10 16:39:40
 * @LastEditors: machun Michael
 * @LastEditTime: 2021-09-10 17:02:26
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

int main(int argc, char *argv[])
{
    int nRet = 0;
    int nVidIndex = 0;
    FILE *pFp = NULL;
    char *pchSrcName = NULL;
    char *pchDstName = NULL;
    char achErrBuf[128] = {0};
    AVFormatContext *ptFmtCtx = NULL;
    AVPacket tAvPkt;

    memset(&tAvPkt, 0, sizeof(tAvPkt));

    do 
    {
        av_log_set_level(AV_LOG_ERROR);

        // 入参格式：程序名 输入文件名 输出文件名
        if (argc < 3)
        {
            av_log(NULL, AV_LOG_ERROR, "input error. usage: exec_prog src_name dst_name\n");
            return -1;
        }

        pchSrcName = argv[1];
        pchDstName = argv[2];
        if (!pchSrcName || !pchDstName)
        {
            av_log(NULL, AV_LOG_ERROR, "input param is null!\n");
            return -1;
        }

        av_log(NULL, AV_LOG_INFO, "input file name:%s, out file name:%s\n", pchSrcName, pchDstName);

        nRet = avformat_open_input(&ptFmtCtx, pchSrcName, NULL, NULL);
        if (0 != nRet)
        {
            av_strerror(nRet, achErrBuf, sizeof(achErrBuf));
            av_log(NULL, AV_LOG_ERROR, "avformat_open_input failed, ret:%d, error:%s\n", nRet, achErrBuf);
            break;
        }

        // 打印输入多媒体文件的元数据
        av_dump_format(ptFmtCtx, 0, pchSrcName, 0);

        // 找到多媒体文件中的视频流索引
        nVidIndex = av_find_best_stream(ptFmtCtx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
        if (nVidIndex < 0)
        {
            av_strerror(nRet, achErrBuf, sizeof(achErrBuf));
            av_log(NULL, AV_LOG_ERROR, "av_find_best_stream video stream failed, ret:%d, error:%s\n", nRet, achErrBuf);
            break;
        }

        // 打开目标文件
        pFp = fopen(pchDstName, "w+");
        if (NULL == pFp)
        {
            av_log(NULL, AV_LOG_ERROR, "fopen %s failed, error:%s\n", pchDstName, strerror(errno));
            break;
        }

        // av packet 初始化
        av_init_packet(&tAvPkt);

        // 读取数据
        while(0 == av_read_frame(ptFmtCtx, &tAvPkt))
        {
            if (tAvPkt.stream_index == nVidIndex)
            {
                fwrite(tAvPkt.data, 1, tAvPkt.size, pFp);
            }

            av_packet_unref(&tAvPkt);
        }
    }while(0);

    if (NULL != ptFmtCtx)
    {
        avformat_close_input(&ptFmtCtx);
    }

    if (NULL != pFp)
    {
        fclose(pFp);
    }

    return 0;
}