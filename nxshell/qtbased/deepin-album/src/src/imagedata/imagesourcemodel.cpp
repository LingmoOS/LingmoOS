// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "imagesourcemodel.h"

/**
   @class ImageSourceModel
   @brief 图片数据模型，提供缩略图/图片展示的图片数据信息。

   @note 此数据模型仅存储需进行展示的图像文件 url 路径列表，
    详细的图像文件信息使用 ImageInfo 进行获取。
 */

ImageSourceModel::ImageSourceModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

ImageSourceModel::~ImageSourceModel() {}

/**
   @return 返回当前数据模型的数据类型及在QML中使用的别名
 */
QHash<int, QByteArray> ImageSourceModel::roleNames() const
{
    return {{Types::ImageUrlRole, "imageUrl"}};
}

/**
   @return 返回数据索引 \a index 和数据类型 \a role 所指向的数据
 */
QVariant ImageSourceModel::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index, CheckIndexOption::ParentIsInvalid | CheckIndexOption::IndexIsValid)) {
        return {};
    }

    switch (role) {
        case Types::ImageUrlRole:
            return imageUrlList.at(index.row());
        default:
            break;
    }

    return {};
}

/**
   @brief 设置数据索引 \a index 和数据类型 \a role 所指向的数据为 \a value
   @return 是否设置数据成功
 */
bool ImageSourceModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!checkIndex(index, CheckIndexOption::ParentIsInvalid | CheckIndexOption::IndexIsValid)) {
        return false;
    }

    switch (role) {
        case Types::ImageUrlRole:
            imageUrlList.replace(index.row(), value.toUrl());

            Q_EMIT dataChanged(index, index);
            return true;
        default:
            break;
    }

    return false;
}

/**
   @return 返回当前数据模型的总行数
 */
int ImageSourceModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return imageUrlList.count();
}

/**
   @return 返回传入文件路径 \a file 在数据模型中的索引，无此文件则返回 -1
 */
int ImageSourceModel::indexForImagePath(const QUrl &file)
{
    if (file.isEmpty()) {
        return -1;
    }

    return imageUrlList.indexOf(file);
}

/**
   @brief 设置图像文件列表，重置模型数据
 */
void ImageSourceModel::setImageFiles(const QList<QUrl> &files)
{
    beginResetModel();
    imageUrlList = files;
    endResetModel();
}

/**
   @brief 从数据模型中移除文件路径 \a fileName 指向的数据
 */
void ImageSourceModel::removeImage(const QUrl &fileName)
{
    int index = imageUrlList.indexOf(fileName);
    if (-1 != index) {
        beginRemoveRows(QModelIndex(), index, index);
        imageUrlList.removeAt(index);
        endRemoveRows();
    }
}
