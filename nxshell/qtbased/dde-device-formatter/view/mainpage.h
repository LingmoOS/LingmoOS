// SPDX-FileCopyrightText: 2016 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MAINPAGE_H
#define MAINPAGE_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QLineEdit>
#include <QVariantAnimation>


class QCheckBox;
class QProgressBar;
class MainPage : public QWidget
{
    Q_OBJECT

public:
    enum FsType {
        Fat16,
        Fat32,
        Ntfs,
    };
    Q_ENUM(FsType)

    explicit MainPage(const QString& defautFormat = "", QWidget *parent = 0);
    void initUI();
    QString selectedFormat();
    void initConnections();
    QString getLabel();

    QString getTargetPath() const;
    void setTargetPath(const QString &targetPath);
    QString formatSize(const qint64& num);
    QString getSelectedFs() const;
    bool shouldErase() const;

    int getMaxLabelNameLength() const;
    void setMaxLabelNameLength(int maxLabelNameLength);

signals:

public slots:
    void onCurrentSelectedTypeChanged(const QString& type);
protected:
    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;

private:
    QLabel* m_iconLabel = NULL;
    QComboBox* m_typeCombo = NULL;
    QPushButton* m_formatButton = NULL;
    QProgressBar* m_storageProgressBar = NULL;
    QStringList m_fileFormat;
    QString m_defautlFormat;
    QLineEdit* m_labelLineEdit = NULL;
    QLabel* m_warnLabel = NULL;
    QString m_targetPath;
    QLabel* m_remainLabel = NULL;
    QLabel* m_nameLabel = NULL;
    int m_maxLabelNameLength = 0;
    QCheckBox* m_quickfmt = nullptr;
};

#endif // MAINPAGE_H
