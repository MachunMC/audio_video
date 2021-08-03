/*
	采集音频的步骤
	1. 注册设备

	2. 设置采集方式

	3. 打开音频设备

	4. 读取音频数据

	备注：如果执行时报找不到库的错误，vi /etc/profile，加上export LD_LIBRARY_PATH="/usr/local/ffmpeg/lib"。
		 或者执行直接export命令，指定ld链接库的路径。修改/etc/profile可以永久生效，执行export只对当前用户本次生效
*/


#include <stdio.h>
extern "C"
{
#include "libavdevice/avdevice.h"
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
}


#define DEV_NAME_MAX_LEN  1024
#define ERR_BUF_MAX_LEN   1024


int main(void)
{
	int nRet = 0;
	int nRdCnt = 0;
	AVPacket tAvPkt;
	AVInputFormat* ptAudFmt = NULL;
	AVFormatContext* ptAvFmtCtx = NULL;
	char s8DevName[DEV_NAME_MAX_LEN] = { 0 };
	char s8ErrBuf[ERR_BUF_MAX_LEN] = { 0 };

	// 0. 设置打印等级
	av_log_set_level(AV_LOG_DEBUG);

	// 1. 注册设备
	avdevice_register_all();

	// 2. 设置采集方式，linux下采集方式为alsa，mac下为avfoundation，windows为dshow
	ptAudFmt = av_find_input_format("alsa");
	if (NULL == ptAudFmt)
	{
		av_log(NULL, AV_LOG_DEBUG, "av_find_input_format alsa failed\n");
		//printf("av_find_input_format alsa failed\n");
		return -1;
	}

	// 3. 打开音频设备，注意这里的设备名称，不同平台下格式不一样
	strncpy(s8DevName, "hw:0, 0", sizeof(s8DevName));

	// 与avformat_close_input成对使用，否则会造成内存泄漏
	nRet = avformat_open_input(&ptAvFmtCtx, s8DevName, ptAudFmt, NULL);
	if (0 != nRet)
	{
		av_strerror(nRet, s8ErrBuf, ERR_BUF_MAX_LEN);
		//printf("avformat_open_input failed, error %s\n", s8ErrBuf);
		av_log(NULL, AV_LOG_DEBUG, "avformat_open_input failed, error %s\n", s8ErrBuf);
		return -1;
	}
	else
	{
		av_log(NULL, AV_LOG_DEBUG, "avformat_open_input succ\n");
		//printf("avformat_open_input succ\n");
	}

	// 4. 读取音频数据
	av_init_packet(&tAvPkt);    // 初始化变量，和av_packet_unref成对使用

	while (1)
	{
		if (nRdCnt > 100)
		{
			break;
		}

		nRet = av_read_frame(ptAvFmtCtx, &tAvPkt);
		if (nRet != 0)
		{
			av_strerror(nRet, s8ErrBuf, ERR_BUF_MAX_LEN);
			av_log(NULL, AV_LOG_DEBUG, "av_read_frame failed, error %s\n", s8ErrBuf);
			//printf("av_read_frame failed, error %s\n", s8ErrBuf);
			break;
		}
		else
		{
			av_packet_unref(&tAvPkt); // 注意:每次read后，都需要释放资源
			//printf("read succ. frame_size:%d\n", tAvPkt.size);
			av_log(NULL, AV_LOG_DEBUG, "read succ. frame_size:%d\n", tAvPkt.size);
		}

		nRdCnt++;
	}
	
	av_packet_unref(&tAvPkt);			// 销毁资源
	avformat_close_input(&ptAvFmtCtx);  // 销毁上下文

	return 0;
}