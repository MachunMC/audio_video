/*
 * @Descripttion: YUV数据处理。参考雷宵骅博客：https://blog.csdn.net/leixiaohua1020/article/details/50534150
 * @version: 
 * @Author: machun Michael
 * @Date: 2021-09-03 14:45:00
 * @LastEditors: machun Michael
 * @LastEditTime: 2021-09-07 16:03:18
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define WORK_PATH "/home/machun/ffmpeg_learn/vs_space"

int yuv420p_split(char *pchUrl, int nWidth, int nHight);
int yuv422p_split(char *pchUrl, int nWidth, int nHight);
int yuv444p_split(char *pchUrl, int nWidth, int nHight);
int yuv420p_gray(char *pchUrl, int nWidth, int nHight);
int yuv420p_luma_halve(char *pchUrl, int nWidth, int nHight);
int yuv420p_border(char *pchUrl, int nWidth, int nHight, int nBorder);

int main()
{
    int nRet = 0;

    // nRet = yuv420p_split(WORK_PATH"/lena_256x256_yuv420p.yuv", 256, 256);
    // if (0 != nRet)
    // {
    //     printf("yuv420p_split failed, ret:%d\n", nRet);
    // }

    // nRet = yuv422p_split(WORK_PATH"/lena_256x256_yuv422p.yuv", 256, 256);
    // if (0 != nRet)
    // {
    //     printf("yuv422p_split failed, ret:%d\n", nRet);
    // }

    // nRet = yuv444p_split(WORK_PATH"/lena_256x256_yuv444p.yuv", 256, 256);
    // if (0 != nRet)
    // {
    //     printf("yuv444p_split failed, ret:%d\n", nRet);
    // }

    // nRet = yuv420p_gray(WORK_PATH"/lena_256x256_yuv420p.yuv", 256, 256);
    // if (0 != nRet)
    // {
    //     printf("yuv420p_gray failed, ret:%d\n", nRet);
    // }

    // nRet = yuv420p_luma_halve(WORK_PATH"/lena_256x256_yuv420p.yuv", 256, 256);
    // if (0 != nRet)
    // {
    //     printf("yuv420p_luma_halve failed, ret:%d\n", nRet);
    // }

    nRet = yuv420p_border(WORK_PATH"/lena_256x256_yuv420p.yuv", 256, 256, 20);
    if (0 != nRet)
    {
        printf("yuv420p_border failed, ret:%d\n", nRet);
    }

    return 0;
}

/**
 * @description: 分离YUV 420p像素数据中的Y、U、V分量，分别保存成三个文件，yuv 420p格式，先存储所有像素点的Y分量，在存储U分量，最后存储V分量
 * @param {char} *pchUrl，YUV文件路径
 * @param {int} nWidth，YUV文件的宽
 * @param {int} nHight，YUV文件的高
 * @return {*}，成功返回0，失败返回-1
 */
int yuv420p_split(char *pchUrl, int nWidth, int nHight)
{
    FILE *pFpRaw = fopen(pchUrl, "r");
    FILE *pFpY = fopen(WORK_PATH"/420p_out_y.y", "w+");
    FILE *pFpU = fopen(WORK_PATH"/420p_out_u.y", "w+");
    FILE *pFpV = fopen(WORK_PATH"/420p_out_v.y", "w+");
    unsigned char *pbyBuf = (unsigned char*)malloc(nWidth * nHight * 3 / 2 + 1); 
    
    fread(pbyBuf, nWidth * nHight * 3 / 2, 1, pFpRaw);

    // Y分量
    fwrite(pbyBuf, nWidth * nHight, 1, pFpY);

    // U分量
    fwrite(pbyBuf + nWidth * nHight, nWidth * nHight/4, 1, pFpU);

    // V分量
    fwrite(pbyBuf + nWidth * nHight * 5/4, nWidth * nHight/4, 1, pFpV);

    free(pbyBuf);
    fclose(pFpRaw);
    fclose(pFpY);
    fclose(pFpU);
    fclose(pFpV);

    return 0;
}

/**
 * @description: 分离YUV 422p像素数据中的Y、U、V分量，分别保存成三个文件，yuv 422p格式，先存储所有像素点的Y分量，在存储U分量，最后存储V分量
 * @param {char} *pchUrl，YUV文件路径
 * @param {int} nWidth，YUV文件的宽
 * @param {int} nHight，YUV文件的高
 * @return {*}，成功返回0，失败返回-1
 */
int yuv422p_split(char *pchUrl, int nWidth, int nHight)
{
    FILE *pFpRaw = fopen(pchUrl, "r");
    FILE *pFpY = fopen(WORK_PATH"/422p_out_y.y", "w+");
    FILE *pFpU = fopen(WORK_PATH"/422p_out_u.y", "w+");
    FILE *pFpV = fopen(WORK_PATH"/422p_out_v.y", "w+");
    unsigned char *pbyBuf = (unsigned char*)malloc(nWidth * nHight * 2 + 1);
    
    fread(pbyBuf, nWidth * nHight * 2, 1, pFpRaw);

    // Y分量
    fwrite(pbyBuf, nWidth * nHight, 1, pFpY);

    // U分量
    fwrite(pbyBuf + nWidth * nHight, nWidth * nHight/2, 1, pFpU);

    // V分量
    fwrite(pbyBuf + 3/2 * nWidth * nHight, nWidth * nHight/2, 1, pFpV);

    free(pbyBuf);
    fclose(pFpRaw);
    fclose(pFpY);
    fclose(pFpU);
    fclose(pFpV);

    return 0;
}

/**
 * @description: 分离YUV 444p像素数据中的Y、U、V分量，分别保存成三个文件，yuv 444p格式，先存储所有像素点的Y分量，在存储U分量，最后存储V分量
 * @param {char} *pchUrl，YUV文件路径
 * @param {int} nWidth，YUV文件的宽
 * @param {int} nHight，YUV文件的高
 * @return {*}，成功返回0，失败返回-1
 */
int yuv444p_split(char *pchUrl, int nWidth, int nHight)
{
    FILE *pFpRaw = fopen(pchUrl, "r");
    FILE *pFpY = fopen(WORK_PATH"/444p_out_y.y", "w+");
    FILE *pFpU = fopen(WORK_PATH"/444p_out_u.y", "w+");
    FILE *pFpV = fopen(WORK_PATH"/444p_out_v.y", "w+");
    unsigned char *pbyBuf = (unsigned char*)malloc(nWidth * nHight * 3 + 1);
    
    fread(pbyBuf, nWidth * nHight * 3, 1, pFpRaw);

    // Y分量
    fwrite(pbyBuf, nWidth * nHight, 1, pFpY);

    // U分量
    fwrite(pbyBuf + nWidth * nHight, nWidth * nHight, 1, pFpU);

    // V分量
    fwrite(pbyBuf + 2 * nWidth * nHight, nWidth * nHight, 1, pFpV);

    free(pbyBuf);
    fclose(pFpRaw);
    fclose(pFpY);
    fclose(pFpU);
    fclose(pFpV);

    return 0;
}

/**
 * @description: 去掉yuv 420p数据中的颜色
 * @param {char} *pchUrl，文件路径
 * @param {int} nWidth，宽
 * @param {int} nHight，高
 * @return {*}，成功返回0，失败返回-1
 */
int yuv420p_gray(char *pchUrl, int nWidth, int nHight)
{
    FILE *pFpRaw = fopen(pchUrl, "r");
    FILE *pFpNew = fopen(WORK_PATH"/420p_out_gray.yuv", "w+");
    unsigned char *pbyBuf = (unsigned char*)malloc(nWidth * nHight * 3/2 + 1);

    fread(pbyBuf, nWidth * nHight * 3/2, 1, pFpRaw);

    // 这里为什么要将UV分量置为128？
    // 这是因为U、V是图像中的经过偏置处理的色度分量。色度分量在偏置处理前的取值范围是-128至127，这时候的无色对应的是0。
    // 经过偏置后色度分量取值变成了0至255，因而此时的无色对应的就是128了
    memset(pbyBuf + nWidth * nHight, 128, nWidth * nHight/2); // 去掉UV分量
    fwrite(pbyBuf, nWidth * nHight * 3/2, 1, pFpNew);

    fclose(pFpRaw);
    fclose(pFpNew);

    return 0;
}

/**
 * @description: 将yuv 420p图片的亮度减半
 * @param {char} *pchUrl，文件路径
 * @param {int} nWidth，宽
 * @param {int} nHight，高
 * @return {*}，成功返回0，失败返回-1
 */
int yuv420p_luma_halve(char *pchUrl, int nWidth, int nHight)
{
    FILE *pFpRaw = fopen(pchUrl, "r");
    FILE *pFpNew = fopen(WORK_PATH"/420p_out_luma_halve.yuv", "w+");
    unsigned char *pbyBuf = (unsigned char*)malloc(nWidth * nHight * 3/2 + 1);

    fread(pbyBuf, nWidth * nHight * 3/2, 1, pFpRaw);

    // 将Y分量的值减半，即可将亮度减半
    for(int i = 0; i < nWidth * nHight; i++)
    {
        pbyBuf[i] = pbyBuf[i] / 2;
    }
    fwrite(pbyBuf, nWidth * nHight * 3/2, 1, pFpNew);

    fclose(pFpRaw);
    fclose(pFpNew);

    return 0;
}

/**
 * @description: 将yuv 420p图片加上一个边框
 * @param {char} *pchUrl，文件路径
 * @param {int} nWidth，宽
 * @param {int} nHight，高
 * @param {int} nBorder，边框宽度
 * @return {*}，成功返回0，失败返回-1
 */
int yuv420p_border(char *pchUrl, int nWidth, int nHight, int nBorder)
{
    FILE *pFpRaw = fopen(pchUrl, "r");
    FILE *pFpNew = fopen(WORK_PATH"/420p_out_border.yuv", "w+");
    unsigned char *pbyBuf = (unsigned char*)malloc(nWidth * nHight * 3/2 + 1);

    fread(pbyBuf, nWidth * nHight * 3/2, 1, pFpRaw);

    // 一行一行处理数据，将nBorder范围内的数据，亮度调到最大，即Y分量置为255
    for (int i = 0; i < nHight; i++)
    {
        for (int j = 0; j < nWidth; j++)
        {
            if (i < nBorder || i > (nHight - nBorder) || j < nBorder || j > (nWidth - nBorder))
            {
                pbyBuf[i*nWidth+j] = 255;
            }
        }
    }

    fwrite(pbyBuf, nWidth * nHight * 3/2, 1, pFpNew);

    fclose(pFpRaw);
    fclose(pFpNew);

    return 0;
}