/*
 * @Descripttion: 
 * @version: 
 * @Author: machun Michael
 * @Date: 2021-09-29 13:49:09
 * @LastEditors: machun Michael
 * @LastEditTime: 2021-09-29 14:21:19
 */
#include <stdio.h>
#include <stdlib.h>
#include "libavutil/log.h"
#include "libavutil/error.h"
#include "libavformat/avformat.h"

int main(int argc, char *argv[])
{
    int nRet = 0;
    AVFormatContext *ptFmtCtx = NULL;
    AVDictionaryEntry *ptPreDicEntry = NULL;
    
    // 设置log等级
    // av_log_set_level(AV_LOG_DEBUG);

    if (argc < 2)
    {
        printf("Usage:%s input_file_name.\nExample to show how to get metadata of the input media file.\n", argv[0]);
        return -1;
    }

    do 
    {
        // 打开多媒体文件
        nRet = avformat_open_input(&ptFmtCtx, argv[1], NULL, NULL);
        if (0 != nRet)
        {
            printf("avformat_open_input failed, ret:%d, error:%s\n", nRet, av_err2str(nRet));
            break;
        }

        // 尝试读取并解码部分帧，来获取部分缺失信息（完善avformat_open_input获取到的数据）
        nRet = avformat_find_stream_info(ptFmtCtx, NULL);
        if (0 != nRet)
        {
            printf("avformat_find_stream_info failed, ret:%d, error:%s\n", nRet, av_err2str(nRet));
            break;
        }

        // 获取metadata数量
        printf("metadata count:%d\n", av_dict_count(ptFmtCtx->metadata));

        // 遍历所有metadata
        while(ptPreDicEntry = av_dict_get(ptFmtCtx->metadata, "", ptPreDicEntry, AV_DICT_IGNORE_SUFFIX))
        {
            printf("%s:%s\n", ptPreDicEntry->key, ptPreDicEntry->value);
        }
    }while(0);

    if (NULL != ptFmtCtx)
    {
        avformat_close_input(&ptFmtCtx);
    }

    return 0;
}