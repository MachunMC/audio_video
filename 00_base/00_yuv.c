/*
 * @Descripttion: YUV数据处理
 * @version: 
 * @Author: machun Michael
 * @Date: 2021-09-03 14:45:00
 * @LastEditors: machun Michael
 * @LastEditTime: 2021-09-06 17:30:54
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define WORK_PATH "/home/machun/ffmpeg_learn/vs_space"

int yuv420p_split(char *pchUrl, int nWidth, int nHight, int nNum);


int main()
{
    int nRet = 0;

    // 分离YUV 420p数据中的Y、U、V分量
    nRet = yuv420p_split(WORK_PATH"/lena_256x256_yuv420p.yuv", 256, 256, 1);
    if (0 != nRet)
    {
        printf("yuv420p_split failed, ret:%d\n", nRet);
    }
    else
    {
        printf("yuv420p_split succ!\n");
    }

    return 0;
}

/**
 * @description: 分离YUV 420p像素数据中的Y、U、V分量，分别保存成三个文件，yuv 420p格式，先存储所有像素点的Y分量，在存储U分量，最后存储V分量
 * @param {char} *pchUrl，YUV文件路径
 * @param {int} nWidth，YUV文件的宽
 * @param {int} nHight，YUV文件的高
 * @param {int} nNum，需要处理的帧数
 * @return {*}
 */
int yuv420p_split(char *pchUrl, int nWidth, int nHight, int nNum)
{
    FILE *pFpRaw = NULL;
    FILE *pFpY = NULL;
    FILE *pFpU = NULL;
    FILE *pFpV = NULL;
    char *pchBuf = NULL;
    int nBufLen = nWidth * nHight * 3 / 2;  // yuv 420p数据量是原来的1/2，*3表示Y、U、V三个分量
    char achPath[256] = {0};

    if (NULL == pchUrl)
    {
        printf("input null param\n");
        return -1;
    }

    pFpRaw = fopen(pchUrl, "r");
    if (NULL == pFpRaw)
    {
        printf("fopen %s failed, error:%s\n", pchUrl, strerror(errno));
        return -1;
    }

    memset(achPath, 0, sizeof(achPath));
    snprintf(achPath, sizeof(achPath), "%s/out_y.y", WORK_PATH);
    pFpY = fopen(achPath, "w+");
    if (NULL == pFpY)
    {
        printf("fopen %s failed, error:%s\n", achPath, strerror(errno));
        return -1;
    }

    memset(achPath, 0, sizeof(achPath));
    snprintf(achPath, sizeof(achPath), "%s/out_u.y", WORK_PATH);
    pFpU = fopen(achPath, "w+");
    if (NULL == pFpY)
    {
        printf("fopen %s failed, error:%s\n", achPath, strerror(errno));
        return -1;
    }

    memset(achPath, 0, sizeof(achPath));
    snprintf(achPath, sizeof(achPath), "%s/out_v.y", WORK_PATH);
    pFpV = fopen(achPath, "w+");
    if (NULL == pFpY)
    {
        printf("fopen %s failed, error:%s\n", achPath, strerror(errno));
        return -1;
    }

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
        fwrite(pchBuf, nWidth * nHight, 1, pFpY);

        // U分量
        fwrite(pchBuf + nWidth * nHight, nWidth * nHight/4, 1, pFpU);

        // V分量
        fwrite(pchBuf + nWidth * nHight * 5/4, nWidth * nHight/4, 1, pFpV);
    }

    fclose(pFpRaw);
    fclose(pFpY);
    fclose(pFpU);
    fclose(pFpV);

    return 0;
}