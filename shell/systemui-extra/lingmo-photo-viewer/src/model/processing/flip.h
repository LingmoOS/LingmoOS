#ifndef FLIP_H
#define FLIP_H

#include "processingbase.h"

class Flip : public ProcessingBase
{

public:
    Mat processing(const Mat &mat, const QVariant &args); //旋转

private:
    Mat rotateRight(const Mat &mat); //顺时针旋转
    Mat rotateLeft(const Mat &mat);  //顺时针旋转
    Mat flipH(const Mat &mat);       //水平翻转
    Mat flipV(const Mat &mat);       //垂直翻转
};

#endif // FLIP_H
