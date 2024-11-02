#include "flip.h"

Mat Flip::processing(const Mat &mat, const QVariant &args)
{
    int type = args.toInt();
    switch (type) {
    case FlipWay::clockwise:
        return rotateRight(mat);
    case FlipWay::counterclockwise:
        return rotateLeft(mat);
    case FlipWay::horizontal:
        return flipH(mat);
    case FlipWay::vertical:
        return flipV(mat);
    }

    return mat;
}

Mat Flip::rotateRight(const Mat &mat)
{
    Mat temp, dst;
    transpose(mat, temp);
    flip(temp, dst, 1);
    return dst;
}

Mat Flip::rotateLeft(const Mat &mat)
{
    Mat temp, dst;
    transpose(mat, temp);
    flip(temp, dst, 0);
    return dst;
}

Mat Flip::flipH(const Mat &mat)
{
    Mat dst;
    flip(mat, dst, 1);
    return dst;
}

Mat Flip::flipV(const Mat &mat)
{
    Mat dst;
    flip(mat, dst, 0);
    return dst;
}
