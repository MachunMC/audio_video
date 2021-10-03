/*
 * @Descripttion: 
 * @version: 
 * @Author: machun Michael
 * @Date: 2021-09-29 15:04:00
 * @LastEditors: machun Michael
 * @LastEditTime: 2021-10-03 16:53:28
 */
#include <stdio.h>
#include <stdlib.h>
#include "libavutil/log.h"
#include "libavutil/error.h"
#include "libavutil/imgutils.h"
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"

uint8_t *g_pbyVideoDstData[4] = {NULL};
int g_nVideoBufSize = 0;
int g_anLineSize[4] = {0};
FILE *g_pFpVideoDst = NULL;
FILE *g_pFpAudioDst = NULL;

// 音视频数据解码
int decode_packet(AVCodecContext *ptCodecCtx, AVPacket *ptPkt, AVFrame *ptFrame)
{
    int nRet = 0;
    static int s_nVideoFrameCnt = 0;
    static int s_nAudioFrameCnt = 0;
    size_t unpadded_linesize = 0;

    if (!ptCodecCtx || !ptPkt || !ptFrame)
    {
        printf("input param is NULL\n");
        return -1;
    }

    // 数据送给解码器解码
    nRet = avcodec_send_packet(ptCodecCtx, ptPkt);
    if (0 != nRet)
    {
        printf("avcodec_send_packet failed\n");
        return -1;
    }

    // 接收解码后的数据
    while(nRet >= 0)
    {
        nRet = avcodec_receive_frame(ptCodecCtx, ptFrame);
        if (nRet < 0)
        {
            if (nRet == AVERROR_EOF || nRet == AVERROR(EAGAIN))
            {
                return 0;
            }
            else
            {
                printf("avcodec_receive_frame failed, error:%s\n", av_err2str(nRet));
                return nRet;
            }
        }

        if (ptCodecCtx->codec->type == AVMEDIA_TYPE_VIDEO)
        {
            printf("video_fram:%d\n", s_nVideoFrameCnt++);
            
            /* copy decoded frame to destination buffer: this is required since rawvideo expects non aligned data */
            av_image_copy(g_pbyVideoDstData, g_anLineSize, (const uint8_t **)ptFrame->data, ptFrame->linesize, ptCodecCtx->pix_fmt, ptCodecCtx->width, ptCodecCtx->height);
            fwrite(g_pbyVideoDstData[0], 1, g_nVideoBufSize, g_pFpVideoDst);
        }
        else if (ptCodecCtx->codec->type == AVMEDIA_TYPE_AUDIO)
        {
            printf("audio_fram:%d\n", s_nAudioFrameCnt++);

            unpadded_linesize = ptFrame->nb_samples * av_get_bytes_per_sample(ptFrame->format);
            fwrite(ptFrame->extended_data[0], 1, unpadded_linesize, g_pFpAudioDst);
        }
        else
        {
            continue;
        }

        // 释放资源，防止内存泄露
        av_frame_unref(ptFrame);
    }

    return 0;
}

// 找到音视频流，并打开相应的解码器
int open_codec(AVFormatContext *ptFmtCtx, enum AVMediaType type, int *pnStreamIndex, AVCodecContext **ptCodecCtx)
{
    int nRet = 0;
    AVStream *ptStream = NULL;
    const AVCodec *ptCodec = NULL;

    if (!ptFmtCtx || !pnStreamIndex || !ptCodecCtx)
    {
        printf("input param is NULL\n");
        return -1;
    }
                        
    // 找到音视频流
    nRet = av_find_best_stream(ptFmtCtx, type, -1, -1, NULL, 0);
    if (nRet < 0)
    {
        printf("av_find_best_stream %s failed, ret:%d\n", av_get_media_type_string(type), nRet);
        return -1;
    }
    else
    {
        *pnStreamIndex = nRet; // stream索引
        ptStream = ptFmtCtx->streams[*pnStreamIndex];

        // 找到对应的解码器
        ptCodec = avcodec_find_decoder(ptStream->codecpar->codec_id);
        if (!ptCodec)
        {
            printf("avcodec_find_decoder failed\n");
            return -1;
        }

        // 分配解码器上下文，并初始化上下文默认值
        *ptCodecCtx = avcodec_alloc_context3(ptCodec);
        if (!*ptCodecCtx)
        {
            printf("avcodec_alloc_context3 failed\n");
            return -1;
        }

        // 上下文参数转换到AVFormatContext中stream对应的结构体中
        nRet = avcodec_parameters_to_context(*ptCodecCtx, ptFmtCtx->streams[*pnStreamIndex]->codecpar);
        if (nRet < 0)
        {
            printf("avcodec_parameters_to_context failed\n");
            return -1;
        }

        // 打开解码器
        nRet = avcodec_open2(*ptCodecCtx, ptCodec, NULL);
        if (nRet != 0)
        {
            printf("avcodec_open2 failed\n");
            return -1;
        }
    }

    return 0;
}

int main(int argc, char **argv)
{
    int nRet = 0;
    int nVideoStreamIndex = 0;
    int nAudioStreamIndex = 0;
    char *pchSrcFile = NULL;
    char *pchDstVidFile = NULL;
    char *pchDstAudFile = NULL;
    AVFormatContext *ptFmtCtx = NULL;
    AVCodecContext  *ptVideoCodecCtx = NULL;
    AVCodecContext  *ptAudioCodecCtx = NULL;
    AVStream *ptVideoStream = NULL;
    AVStream *ptAudioStream = NULL;
    AVPacket *ptPkt = NULL;
    AVFrame *ptFrame = NULL;

    if (argc != 4)
    {
        printf("input param error!\nUsage:%s input_src_file output_video_file output_audio_file\n", argv[0]);
        return 0;
    }
    
    do 
    {
        pchSrcFile = argv[1];
        pchDstVidFile = argv[2];
        pchDstAudFile = argv[3];

        // 打开文件，并分配上下文
        nRet = avformat_open_input(&ptFmtCtx, pchSrcFile, NULL, NULL);
        if (nRet)
        {
            printf("avformat_open_input failed, ret:%d error:%s\n", nRet, av_err2str(nRet));
            return -1;
        }

        // 进一步获取多媒体文件信息
        nRet = avformat_find_stream_info(ptFmtCtx, NULL);
        if (nRet)
        {
            printf("avformat_find_stream_info failed, ret:%d error:%s\n", nRet, av_err2str(nRet));
            return -1;
        }

        // 输出多媒体文件信息
        av_dump_format(ptFmtCtx, 0, pchSrcFile, 0);

        // 打开视频编码器
        nRet = open_codec(ptFmtCtx, AVMEDIA_TYPE_VIDEO, &nVideoStreamIndex, &ptVideoCodecCtx);
        if (!nRet)
        {
            ptVideoStream = ptFmtCtx->streams[nVideoStreamIndex];

            // 分配编码空间
            nRet = av_image_alloc(g_pbyVideoDstData, g_anLineSize, ptVideoCodecCtx->width, ptVideoCodecCtx->height, ptVideoCodecCtx->pix_fmt, 1);
            if (nRet < 0)
            {
                printf("av_image_alloc failed\n");
                return -1;
            }
            g_nVideoBufSize = nRet;

            // 打开文件
            g_pFpVideoDst = fopen(pchDstVidFile, "wb");
            if (!g_pFpVideoDst)
            {
                printf("fopen %s failed\n", pchDstVidFile);
                return -1;
            }
        }

        // 打开音频编码器
        nRet = open_codec(ptFmtCtx, AVMEDIA_TYPE_AUDIO, &nAudioStreamIndex, &ptAudioCodecCtx);
        if (!nRet)
        {
            ptAudioStream = ptFmtCtx->streams[nAudioStreamIndex];

            // 打开文件
            g_pFpAudioDst = fopen(pchDstAudFile, "wb");
            if (!g_pFpAudioDst)
            {
                printf("fopen %s failed\n", pchDstAudFile);
                return -1;
            }
        }

        ptFrame = av_frame_alloc();
        if (!ptFrame)
        {
            printf("av_frame_alloc failed\n");
            return -1;
        }

        ptPkt = av_packet_alloc();
        if (!ptPkt)
        {
            printf("av_packet_alloc failed\n");
            return -1;
        }
        
        // 读取数据帧
        while(av_read_frame(ptFmtCtx, ptPkt) >= 0)
        {
            // 视频帧
            if (ptPkt->stream_index == nVideoStreamIndex)
            {
                decode_packet(ptVideoCodecCtx, ptPkt, ptFrame);
            }
            else if (ptPkt->stream_index == nAudioStreamIndex) // 音频帧
            {
                decode_packet(ptAudioCodecCtx, ptPkt, ptFrame);
            }
            else // 其他
            {
                continue;
            }

            av_packet_unref(ptPkt);
        }
    }while(0);

    // 释放资源
    if (ptFmtCtx)
    {
        avformat_close_input(&ptFmtCtx);
    }

    if (ptFrame)
    {
        av_frame_free(&ptFrame);
    }

    if (ptPkt)
    {
        av_packet_free(&ptPkt);
    }

    if (g_pFpVideoDst)
    {
        fclose(g_pFpVideoDst);
    }

    if (g_pFpAudioDst)
    {
        fclose(g_pFpAudioDst);
    }

    return 0;
}