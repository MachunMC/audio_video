/*
 * @Descripttion: 
 * @version: 
 * @Author: machun Michael
 * @Date: 2021-09-03 14:45:00
 * @LastEditors: machun Michael
 * @LastEditTime: 2021-09-03 16:16:43
 */
/*
 * @Descripttion: YUV数据处理
 * @version: 
 * @Author: machun Michael
 * @Date: 2021-09-03 14:45:00
 * @LastEditors: machun Michael
 * @LastEditTime: 2021-09-03 14:52:25
 */

#include <stdio.h>
#include <stdlib.h>


int main()
{



    return 0;
}

/**
 * @description: 分离YUV 420p像素数据中的Y、U、V分量，分别保存成三个文件
 * @param {char} *pchUrl，YUV文件路径
 * @param {int} nWidth，YUV文件的宽
 * @param {int} nHight，YUV文件的高
 * @param {int} nNum，需要处理的帧数
 * @return {*}
 */
int yuv420_split(char *pchUrl, int nWidth, int nHight, int nNum)
{
    FILE *pFpRaw = NULL;
    FILE *pFpY = NULL;
    FILE *pFpU = NULL;
    FILE *pFpV = NULL;
    char *pchBuf = NULL;
    int nBufLen = nWidth * nHight / 3 * 2;  // YUV 420P数据量是原来的2/3

    if (NULL == pchUrl)
    {
        printf("input null param\n");
        return -1;
    }

    pFpRaw = fopen(pchUrl, "r");
    if (NULL == pFpRaw)
    {
        printf("fopen %s failed\n", pchUrl);
        return -1;
    }

    pFpY = fopen("/home/machun/ffmpeg_learn/out_y.yuv", "w+");
    if (NULL == pFpY)
    {
        printf("fopen /home/machun/ffmpeg_learn/out_y.yuv failed\n");
        return -1;
    }

    pFpU = fopen("/home/machun/ffmpeg_learn/out_u.yuv", "w+");
    if (NULL == pFpU)
    {
        printf("fopen /home/machun/ffmpeg_learn/out_u.yuv failed\n");
        return -1;
    }

    pFpV = fopen("/home/machun/ffmpeg_learn/out_v.yuv", "w+");
    if (NULL == pFpV)
    {
        printf("fopen /home/machun/ffmpeg_learn/out_v.yuv failed\n");
        return -1;
    }

    // 动态申请内存，用于读取YUV数据
    pchBuf = (char*)malloc(nBufLen + 1);
    if (NULL == pchBuf)
    {
        printf("malloc failed\n");
        return -1;
    }

    for (int i = 0; i < nNum; i++)
    {
        fread(pchBuf, nBufLen, 1, pFpRaw);

        // Y分量
        fwrite(pchBuf, nBufLen, 1, pFpY);
    }

    return 0;
}