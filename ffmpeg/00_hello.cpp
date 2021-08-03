//#include <stdio.h>
//
//// ffmpeg库是C语言库，用C++工程编译时，需要指定是C代码，以便做兼容，不然编译会报找不到符号的错误
//extern "C"
//{
//#include "libavutil/avutil.h"
//#include "libavutil/log.h"
//}
//
//
//int main(void)
//{
//	printf("hello world\n");
//	
//	av_log_set_level(AV_LOG_DEBUG);
//	av_log(NULL, AV_LOG_DEBUG, "hello from av_log\n");
//
//	return 0;
//}