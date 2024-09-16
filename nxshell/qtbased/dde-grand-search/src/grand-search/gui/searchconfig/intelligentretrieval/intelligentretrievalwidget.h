// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef INTELLIGENTRETRIEVALWIDGET_H
#define INTELLIGENTRETRIEVALWIDGET_H

#include <QLabel>
#include <QVBoxLayout>
#include <QTimer>

#include <DWidget>

namespace GrandSearch {
class SwitchWidget;
class DetailCheckBox;
class AutoIndexStatus;
class IntelligentRetrievalWidget : public Dtk::Widget::DWidget
{
    Q_OBJECT
public:
    explicit IntelligentRetrievalWidget(QWidget *parent = nullptr);
public slots:
    void updateState();
    void openAppStore(const QString &app);
protected:
    void updateStatusContent(const QVariantHash &status);
    bool isAnalayzeSupported();
    bool isVectorSupported();
    void checkBoxChanged();
    void setAutoIndex(bool on);
    bool getIndexStatus(QVariantHash &statuts);
signals:

public slots:
private:
    QLabel *m_groupLabel = nullptr;
    QVBoxLayout *m_mainLayout = nullptr;
    DetailCheckBox *m_semantic = nullptr;
    DetailCheckBox *m_vector = nullptr;
    QLabel *m_vectorDetail = nullptr;
    SwitchWidget *m_enableIndex = nullptr;
    AutoIndexStatus *m_indexStatus = nullptr;
    QTimer m_timer;
    bool m_ignoreSigal = false;
};

}

#endif // INTELLIGENTRETRIEVALWIDGET_H
