/*
 * @Descripttion: 
 * @version: 
 * @Author: machun Michael
 * @Date: 2021-09-08 19:27:54
 * @LastEditors: machun Michael
 * @LastEditTime: 2021-09-08 19:36:51
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 不加这个宏，编译会报错
#define __STDC_CONSTANT_MACROS 

extern "C"
{
    #include "libavutil/log.h"
}

int main()
{
    char achBuf[128] = "Hello world!";

    av_log_set_level(AV_LOG_INFO);

    av_log(NULL, AV_LOG_ERROR, "error log, %s\n", achBuf);
    av_log(NULL, AV_LOG_INFO, "info log, %s\n", achBuf);
    av_log(NULL, AV_LOG_DEBUG, "debug log, %s\n", achBuf);

    return 0;
}