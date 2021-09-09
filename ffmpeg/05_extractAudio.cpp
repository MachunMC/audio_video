/*
 * @Descripttion: 抽取多媒体文件中的音频数据
 * @version: 
 * @Author: machun Michael
 * @Date: 2021-09-09 10:55:51
 * @LastEditors: machun Michael
 * @LastEditTime: 2021-09-09 14:57:51
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define __STDC_CONSTANT_MACROS 
extern "C"
{
    #include "libavutil/log.h"
    #include "libavutil/error.h"
    #include "libavformat/avformat.h"
}

#define ADTS_HEADER_LEN 7

int AddAdtsHeader(char *pchHeader, int nFrameLenth);

int main(int argc, char *argv[])
{
    int nRet = 0;
    int nAudIndex = 0;
    int nWrLen = 0;
    char *pchSrcName = NULL;
    char *pchDstName = NULL;
    char achErrBuf[128] = {0};
    char achAdtsHeader[ADTS_HEADER_LEN] = {0};
    FILE *pFpDst = NULL;
    AVFormatContext *ptFmtCtx = NULL;
    AVPacket tAvPkt;

    memset(&tAvPkt, 0, sizeof(tAvPkt));

    do 
    {
        // 设置打印等级
        av_log_set_level(AV_LOG_DEBUG);

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

        // 打开输入文件
        nRet = avformat_open_input(&ptFmtCtx, pchSrcName, NULL, NULL);
        if (0 != nRet)
        {
            av_strerror(nRet, achErrBuf, sizeof(achErrBuf));
            av_log(NULL, AV_LOG_ERROR, "avformat_open_input failed, ret:%d, error:%s\n", nRet, achErrBuf);
            return -1;
        }

        // 打印输入文件元数据
        av_dump_format(ptFmtCtx, 0, pchSrcName, 0);

        pFpDst = fopen(pchDstName, "w+");
        if (NULL == pFpDst)
        {
            av_log(NULL, AV_LOG_ERROR, "fopen %s failed, error:%s\n", pchDstName, strerror(errno));
            return -1;
        }

        // 找到音频流索引值
        nAudIndex = av_find_best_stream(ptFmtCtx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
        if (nAudIndex < 0)
        {
            av_strerror(nAudIndex, achErrBuf, sizeof(achErrBuf));
            av_log(NULL, AV_LOG_ERROR, "av_find_best_stream failed, ret:%d, error:%s\n", nAudIndex, achErrBuf);
            return -1;
        }

        // 包初始化
        av_init_packet(&tAvPkt);

        while(0 == av_read_frame(ptFmtCtx, &tAvPkt))
        {
            // 判断是否为音频流
            if (tAvPkt.stream_index == nAudIndex)
            {
                // 需要添加adts头，否则播放不出来
                AddAdtsHeader(achAdtsHeader, tAvPkt.size);
                fwrite(achAdtsHeader, 1, 7, pFpDst);

                // 写入音频数据
                nWrLen = fwrite(tAvPkt.data, 1, tAvPkt.size, pFpDst);
                if (nWrLen != tAvPkt.size)
                {
                    av_log(NULL, AV_LOG_ERROR, "fwrite succ %d bytes, wanted %d bytes\n", nWrLen, tAvPkt.size);
                }
            }

            av_packet_unref(&tAvPkt);
        }
    }while(0);

    if (NULL != ptFmtCtx)
    {
        avformat_close_input(&ptFmtCtx);
    }

    if (NULL != pFpDst)
    {
        fclose(pFpDst);
    }

    return 0;
}

/**
 * @description: 添加aac adts头
 * @param {char} *pchHeader，buffer
 * @param {int} nFrameLenth，帧长度
 * @return {*}，成功返回0，失败返回-1
 */
// 函数内部位操作有点问题，提取出来的aac数据播放有问题
int AddAdtsHeader(char *pchHeader, int nFrameLenth)
{
    unsigned char byProfile = 2; // AAC LC
    unsigned char byFrequenceIndex = 4; // 44.1KHz
    unsigned char byChannelCfg = 2; // 双声道

    pchHeader[0] = 0xFF;  // syncword (8 bits)
    pchHeader[1] = 0xF0;  // syncword (4 bits) + ID (1 bit) + layer (2 bits)+ protection_absent (1 bit)
    pchHeader[2] = ((byProfile - 1) << 6) | (byFrequenceIndex << 2) | (byChannelCfg >> 2); // profile (2 bits) + sampling_frequency_index (4 bits) + private_bit (1 bit) + channel_config (1 bit)
    pchHeader[3] = ((byChannelCfg & 3) << 6) | (nFrameLenth >> 11);  // channel_config (2 bits) + orininal_copy (1 bit) + home (1 bit) + copyright_id_bit (1 bit) + copyright_id_start (1 bit) + aac_frame_lenth (2 bits) 
    pchHeader[4] = (nFrameLenth >> 3) & 0xFF;    // aac_frame_lenth (8 bits)
    pchHeader[5] = (nFrameLenth << 5) & 0xFF | 0x1F;  // aac_frame_lenth (3 bits) + adts_buf_fullness (5 bits)
    pchHeader[6] = 0xFC;  // adts_buf_fullness (6 bits) + number_of_raw_data_blocks_in_frame (2 bits)

    return 0;
}