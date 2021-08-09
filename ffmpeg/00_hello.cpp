/*
 * @Descripttion: 使用ffmpeg库中的log函数，输出打印
 * @version: 
 * @Author: machun Michael
 * @Date: 2021-08-09 05:02:38
 * @LastEditors: machun Michael
 * @LastEditTime: 2021-08-09 06:13:52
 */

#include <stdio.h>
#include <stdlib.h>

// 不加这个宏，编译会报错
#define __STDC_CONSTANT_MACROS 
// ffmpeg是用C写的，当C++程序中包含该库时，需要明确指定是C相关的符号
extern "C"
{
	#include "libavutil/avutil.h"
	#include "libavutil/log.h"
}

int main(void)
{
	av_log_set_level(AV_LOG_DEBUG);
	av_log(NULL, AV_LOG_DEBUG, "hello ffmpeg\n");

	return 0;
}