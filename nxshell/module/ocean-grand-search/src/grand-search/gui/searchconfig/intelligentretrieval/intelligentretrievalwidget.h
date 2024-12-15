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
class LLMWidget;
class DetailCheckBox;
class AutoIndexStatus;
class IntelligentRetrievalWidget : public Dtk::Widget::DWidget
{
    Q_OBJECT
public:
    static bool isQueryLangSupported();
    static QVariantHash getIndexStatus();
    static bool isUpdateIndex();
    static void setAutoIndex(bool on);

    explicit IntelligentRetrievalWidget(QWidget *parent = nullptr);
    bool onCloseEvent();
public slots:
    void updateState();
    void openAppStore(const QString &app);
protected:
    void updateStatusContent(const QVariantHash &status);
#ifdef VECTOR_SEARCH
    bool isVectorSupported();
#endif
    void updateIndexStatusContent(const QVariantHash &statuts);
    void setFulltextQuery(bool on);
signals:

public slots:
protected slots:
    void checkChanged();
private:
    QVBoxLayout *m_mainLayout = nullptr;
    SwitchWidget *m_semantic = nullptr;
    QLabel *m_detailLabel = nullptr;
    QWidget *m_indexWidget = nullptr;
    QVBoxLayout *m_indexLayout = nullptr;
#ifdef VECTOR_SEARCH
    DetailCheckBox *m_vector = nullptr;
    QLabel *m_vectorDetail = nullptr;
#endif
    SwitchWidget *m_featIndex = nullptr;
    SwitchWidget *m_fullTextIndex = nullptr;
    QLabel *m_fullTextLabel = nullptr;
    LLMWidget *m_llmWidget = nullptr;
    AutoIndexStatus *m_indexStatus = nullptr;
    QTimer m_timer;
    bool m_ignoreSigal = false;
};

}

#endif // INTELLIGENTRETRIEVALWIDGET_H
