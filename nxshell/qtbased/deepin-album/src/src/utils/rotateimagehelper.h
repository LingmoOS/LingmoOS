// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ROTATEIMAGEHELPER_H
#define ROTATEIMAGEHELPER_H

#include <QObject>
#include <QSharedPointer>

class RotateImageHelperData;
class RotateImageHelper : public QObject
{
    Q_OBJECT
public:
    static RotateImageHelper *instance();

    Q_SLOT void rotateImageFile(const QString &path, int angle);
    Q_SLOT void resetRotateState();

    Q_SIGNAL void rotateImageFinished(const QString &path, bool ret);

    static bool rotateImageImpl(const QString &cachePath, const QString &path, int angle);

private:
    explicit RotateImageHelper(QObject *parent = nullptr);
    virtual ~RotateImageHelper() = default;

    void enqueueRotateTask(const QString &path, int angle);
    void checkDataValid();
    // internal 用于异步处理图片旋转状态
    Q_SIGNAL void recordRotateImage(const QString &targetPath);
    Q_SIGNAL void clearRotateStatus(const QString &targetPath);

private:
    QSharedPointer<RotateImageHelperData> data;

    Q_DISABLE_COPY(RotateImageHelper)
};

#endif  // ROTATEIMAGEHELPER_H
