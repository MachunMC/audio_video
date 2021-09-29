/*
 * @Descripttion: 
 * @version: 
 * @Author: machun Michael
 * @Date: 2021-09-29 15:04:00
 * @LastEditors: machun Michael
 * @LastEditTime: 2021-09-29 15:06:38
 */
#include <stdio.h>
#include <libavformat/avformat.h>

int main(int argc, char **argv)
{

// 1. 打开文件
    const char *ifilename = "./wanfeng.mp4";
    printf("in_filename = %s\n", ifilename);
    // AVFormatContext是描述一个媒体文件或媒体流的构成和基本信息的结构体
    AVFormatContext *ifmt_ctx = NULL;           // 输入文件的demux
    // 打开文件，主要是探测协议类型，如果是网络文件则创建网络链接
    int ret = avformat_open_input(&ifmt_ctx, ifilename, NULL, NULL);
    if (ret < 0) {
        char buf[1024] = {0};
        av_strerror(ret, buf, sizeof (buf) - 1);
        printf("open %s failed: %s\n", ifilename, buf);
        return -1;
    }

// 2. 读取码流信息
    ret = avformat_find_stream_info(ifmt_ctx, NULL);
    if (ret < 0)  //如果打开媒体文件失败，打印失败原因
    {
        char buf[1024] = { 0 };
        av_strerror(ret, buf, sizeof(buf) - 1);
        printf("avformat_find_stream_info %s failed:%s\n", ifilename, buf);
        avformat_close_input(&ifmt_ctx);
        return -1;
    }


    // 3.打印总体信息
    printf("\n==== av_dump_format in_filename:%s ===\n", ifilename);
    av_dump_format(ifmt_ctx, 0, ifilename, 0);
    printf("\n==== av_dump_format finish =======\n\n");
    printf("media name:%s\n", ifmt_ctx->url);
    printf("stream number:%d\n", ifmt_ctx->nb_streams); //  nb_streams媒体流数量
    printf("media average ratio:%ldkbps\n",(int64_t)(ifmt_ctx->bit_rate/1024)); //  媒体文件的码率,单位为bps
    // duration: 媒体文件时长，单位微妙
    int total_seconds = (ifmt_ctx->duration) / AV_TIME_BASE;  // 1000us = 1ms, 1000ms = 1秒
    printf("audio duration: %02d:%02d:%02d\n",
           total_seconds / 3600, (total_seconds % 3600) / 60, (total_seconds % 60));
    printf("\n");

// 4.读取码流信息
    // 音频
    int audioindex = av_find_best_stream(ifmt_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
    if (audioindex < 0) {
        printf("av_find_best_stream %s eror.", av_get_media_type_string(AVMEDIA_TYPE_AUDIO));
        return -1;
    }
    AVStream *audio_stream = ifmt_ctx->streams[audioindex];
    printf("----- Audio info:\n");
    printf("index: %d\n", audio_stream->index); // 序列号
    printf("samplarate: %d Hz\n", audio_stream->codecpar->sample_rate); // 采样率
    printf("sampleformat: %d\n", audio_stream->codecpar->format); // 采样格式 AV_SAMPLE_FMT_FLTP:8
    printf("audio codec: %d\n", audio_stream->codecpar->codec_id); // 编码格式 AV_CODEC_ID_MP3:86017 AV_CODEC_ID_AAC:86018
    if (audio_stream->duration != AV_NOPTS_VALUE) {
        int audio_duration = audio_stream->duration * av_q2d(audio_stream->time_base);
        printf("audio duration: %02d:%02d:%02d\n",
               audio_duration / 3600, (audio_duration % 3600) / 60, (audio_duration % 60));
    }

    // 视频
    int videoindex = av_find_best_stream(ifmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    if (videoindex < 0) {
        printf("av_find_best_stream %s eror.", av_get_media_type_string(AVMEDIA_TYPE_VIDEO));
        return -1;
    }
    AVStream *video_stream = ifmt_ctx->streams[videoindex];
    printf("----- Video info:\n");
    printf("index: %d\n", video_stream->index); // 序列号
    printf("fps: %lf\n", av_q2d(video_stream->avg_frame_rate)); // 帧率
    printf("width: %d, height:%d \n", video_stream->codecpar->width, video_stream->codecpar->height);
    printf("video codec: %d\n", video_stream->codecpar->codec_id); // 编码格式 AV_CODEC_ID_H264: 27
    if (video_stream->duration != AV_NOPTS_VALUE) {
        int video_duration = video_stream->duration * av_q2d(video_stream->time_base);
        printf("audio duration: %02d:%02d:%02d\n",
               video_duration / 3600, (video_duration % 3600) / 60, (video_duration % 60));
    }

// 5.提取码流
    AVPacket *pkt = av_packet_alloc();
    int pkt_count = 0;
    int print_max_count = 10;
    printf("\n-----av_read_frame start\n");
    while (1)
    {
        ret = av_read_frame(ifmt_ctx, pkt);
        if (ret < 0) {
            printf("av_read_frame end\n");
            break;
        }

        if(pkt_count++ < print_max_count)
        {
            if (pkt->stream_index == audioindex)
            {
                printf("audio pts: %ld\n", pkt->pts);
                printf("audio dts: %ld\n", pkt->dts);
                printf("audio size: %d\n", pkt->size);
                printf("audio pos: %ld\n", pkt->pos);
                printf("audio duration: %lf\n\n",
                       pkt->duration * av_q2d(ifmt_ctx->streams[audioindex]->time_base));
            }
            else if (pkt->stream_index == videoindex)
            {
                printf("video pts: %ld\n", pkt->pts);
                printf("video dts: %ld\n", pkt->dts);
                printf("video size: %d\n", pkt->size);
                printf("video pos: %ld\n", pkt->pos);
                printf("video duration: %lf\n\n",
                       pkt->duration * av_q2d(ifmt_ctx->streams[videoindex]->time_base));
            }
            else
            {
                printf("unknown stream_index:%d\n", pkt->stream_index);
            }
        }
        av_packet_unref(pkt);
    }

// 6.结束
    if(pkt)
        av_packet_free(&pkt);
    if(ifmt_ctx)
        avformat_close_input(&ifmt_ctx);


    return 0;
}