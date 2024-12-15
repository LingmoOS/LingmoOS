// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef AIMODELSERVICE_H
#define AIMODELSERVICE_H

#include <QObject>
#include <QString>
#include <QScopedPointer>

class AIModelServiceData;
class AIModelService : public QObject
{
    Q_OBJECT

public:
    static AIModelService *instance();

    bool isValid() const;

    enum State { None, Loading, LoadSucc, LoadFailed, NotDetectPortrait, LoadTimeout, Cancel };
    State enhanceState(const QString &filePath);

    enum Error { NoError, FormatError, PixelSizeError, LoadFiledError, NotDetectPortraitError };
    Error modelEnabled(int modelID, const QString &filePath) const;
    QList<QPair<int, QString>> supportModel() const;

    // 图像处理过程控制接口
    QString imageProcessing(const QString &filePath, int modelID, const QImage &image);
    Q_SLOT void reloadImageProcessing(const QString &filePath);
    void resetProcess();
    void cancelProcess(const QString &output);

    Q_SIGNAL void enhanceStart();
    Q_SIGNAL void enhanceReload(const QString &output);
    Q_SIGNAL void enhanceEnd(const QString &source, const QString &output, State state);

    bool isTemporaryFile(const QString &filePath);
    QString sourceFilePath(const QString &filePath);
    QString lastProcOutput() const;

    bool isWaitSave() const;
    void saveFileDialog(const QString &filePath, QWidget *target = nullptr);
    void saveEnhanceFile(const QString &filePath);
    void saveEnhanceFileAs(const QString &filePath, QWidget *target = nullptr);

    bool detectErrorAndNotify(QWidget *targetWidget, Error error, const QString &output = QString::null);

    // 图片切换/中断增强处理等操作时，清理之前的图像增强状态
    Q_SIGNAL void clearPreviousEnhance();

protected:
    void timerEvent(QTimerEvent *e) override;

private:
    AIModelService(QObject *parent = nullptr);
    ~AIModelService() override;

    bool checkFileSavable(const QString &newPath, QWidget *target = nullptr);
    void showWarningDialog(const QString &notify, QWidget *target = nullptr);
    bool saveFile(const QString &filePath, const QString &newPath);
    void saveTemporaryAs(const QString &filePath, const QString &sourcePath, QWidget *target = nullptr);
    QString checkConvertFile(const QString &filePath, const QImage &image) const;

    // DBus
    Q_SLOT void onDBusEnhanceEnd(const QString &output, int error);

private:
    QScopedPointer<AIModelServiceData> dptr;
};

#endif  // AIMODELSERVICE_H
