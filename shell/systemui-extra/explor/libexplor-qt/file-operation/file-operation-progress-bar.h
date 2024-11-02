/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2019, Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Jing Ding <dingjing@kylinos.cn>
 *
 */

#ifndef FILEOPERATIONPROGRESS_H
#define FILEOPERATIONPROGRESS_H
#include <QWidget>
#include <QHBoxLayout>
#include <QListWidget>
#include <QPushButton>
#include <QToolButton>
#include <gio/gio.h>

class ProgressBar;
class OtherButton;
class MainProgressBar;

class FileOperationProgressBar : public QWidget
{
    Q_OBJECT
public:
    static FileOperationProgressBar* getInstance();

    void removeAllProgressbar ();
    ProgressBar* addFileOperation();
    void showProgress (ProgressBar& progress);
    void removeFileOperation(ProgressBar* progress);
    bool isInhibit();

private:
    explicit FileOperationProgressBar(QWidget *parent = nullptr);
    ~FileOperationProgressBar();
    void showMore ();

    // s3 s4
    bool inhibit ();
    void uninhibit ();

protected:
    void showWidgetList(bool show);
#if 0
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
#endif

Q_SIGNALS:
    void pause();
    void start();
    void canceled();

public Q_SLOTS:
    void mainProgressChange(QListWidgetItem *item);
    void showDelay(int msec = 2000);

public:
    bool m_error = false;

private:
    GUnixFDList*            m_fds = nullptr;

    // layout
    QVBoxLayout* m_main_layout = nullptr;
    GDBusConnection* m_dbus_connection = nullptr;

    // widget
    QListWidget* m_list_widget = nullptr;
    ProgressBar* m_current_main = nullptr;
    OtherButton* m_other_progressbar = nullptr;
    MainProgressBar* m_main_progressbar = nullptr;

    QMap<QListWidgetItem*, ProgressBar*>* m_widget_list = nullptr;
    QMap<ProgressBar*, QListWidgetItem*>* m_progress_list = nullptr;
    QList<ProgressBar*>* m_pro_list = nullptr;

    int m_show_items = 2;
    bool m_show_more = false;
    int m_progress_item_height = 62;
    int m_progress_list_heigth = 200;

    // ui
    QPoint m_position;
    int m_progress_size = 0;
    bool m_is_press = false;

    bool m_more = false;
    static FileOperationProgressBar* instance;
};

class ProgressBar : public QWidget
{
    friend FileOperationProgressBar;
    Q_OBJECT
public:
    explicit ProgressBar (QWidget* parent = nullptr);
    void setIcon (const QString& icon);
    QIcon& getIcon();
    bool getStatus();
    float getProgress();
    float getTotalSize();
    QString getFileName();
    bool isPause();
    void setPause();
    void setResume();
    QString elideText (const QFont &font, const int &width, const QString &strInfo);

private:
    ~ProgressBar();

Q_SIGNALS:
    void pause();
    void resume();
    void cancelled();
    void finished(ProgressBar* fop);
    void sendValue(QString&, QIcon&, double);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    bool event(QEvent *event) override;

public Q_SLOTS:
    void onCancelled();
    void updateValue(double);
    void onElementFoundOne (const QString &uri, const qint64 &size);
    void onElementFoundAll ();
    void onFileOperationProgressedOne(const QString &uri, const QString &destUri, const qint64 &size);
    void updateProgress(const QString &srcUri, const QString &destUri, const QString& fIcon, const quint64& current, const quint64& total);
    void onFileOperationProgressedAll();
    void onElementClearOne(const QString &uri);
    void switchToRollbackPage();
    void onStartSync();
    void onFinished();
    void onFileRollbacked(const QString &destUri, const QString &srcUri);

private:

    bool m_has_finished = false;
    bool m_sync = false;

    int m_fix_width = 550;
    int m_fix_height = 62;

    int m_btn_size = 18;
    int m_margin_ud = 2;
    int m_margin_lr = 8;
    int m_icon_size = 32;
    int m_text_height = 20;

    int m_progress_width = 80;
    int m_progress_height = 6;

    int m_percent_width = 20;

    // const
    const float m_icon_x = m_margin_lr;
    const float m_icon_y = (m_fix_height - m_margin_ud * 2 - m_icon_size) / 2 + m_margin_ud;

    const float m_text_x = m_margin_lr * 2 + m_icon_size;
    const float m_text_y = (m_fix_height - m_margin_ud * 2 - m_text_height) / 2 + m_margin_ud;
    const float m_text_w = m_fix_width - m_margin_lr * 6 - m_icon_size - m_btn_size * 2 - m_progress_width - m_percent_width;

    const float m_progress_x = m_margin_lr * 3 + m_icon_size + m_text_w;
    const float m_progress_y = (m_fix_height - m_margin_ud * 2 - m_progress_height) / 2 + m_margin_ud;

    const float m_pause_x = m_margin_lr * 5 + m_icon_size + m_text_w + m_progress_width;
    const float m_pause_x_r = m_pause_x + m_btn_size;
    const float m_pause_y = (m_fix_height - m_margin_ud * 2 - m_btn_size) / 2 + m_margin_ud;
    const float m_pause_y_b = m_pause_y + m_btn_size;

    const float m_close_x = m_pause_x + m_margin_lr + m_btn_size;
    const float m_close_x_r = m_close_x + m_btn_size;
    const float m_close_y = m_pause_y;
    const float m_close_y_b = m_pause_y_b;

    // value
    QIcon m_icon;
    double m_current_value = 0.0;

    QString m_src_uri;
    QString m_dest_uri;
    int m_total_count = 0;
    int m_current_count = 1;
    int m_update_count = 0;
    quint64 m_total_size = 0;
    qint32 m_current_size = 0;

    bool m_pause = false;

    bool m_is_stopping = false;
};

class MainProgressBar : public QWidget
{
    Q_OBJECT
public:
    explicit MainProgressBar(QWidget *parent = nullptr);
    void initPrarm();
    void setFileIcon (QIcon& icon);
    void setTitle (QString title);
    void setPause();
    void setResume();
    void setIsSync(bool);
    void setProgress(float);
    void setFileName(QString);
    QString elideText(const QFont &font, const int &width, const QString &strInfo);
protected:
    void paintEvent(QPaintEvent *event) override;
//    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    bool event(QEvent *event) override;
private:
    void paintFoot (QPainter& painter);
    void paintHeader (QPainter& painter);
    void paintContent (QPainter& painter);
    void paintProgress (QPainter& painter);

Q_SIGNALS:
    void minimized();
    void closeWindow();
    void pause();
    void start();

public Q_SLOTS:
    void cancelld();
    void updateValue (QString&, QIcon&, double);

private:
    bool m_sync = false;
    // can modify
    // header
    QString m_title;
    int m_fix_width = 550;
    int m_fix_height = 176;
    int m_title_width = 480;
    int m_header_height = 30;

    // btn
    int m_btn_margin_top = 8;
    int m_btn_margin = 3;
    int m_btn_size = 32;

    // btn
    QPushButton* m_btn_close = nullptr;
    QPushButton* m_btn_mini = nullptr;
    QToolButton* m_btn_pause = nullptr;

    // icon
    int m_icon_margin = 20;
    int m_icon_size = 64;

    // file name
    int m_file_name_height = 60;
    int m_file_name_margin = 10;

    // pase button
    int m_pause_btn_height = 20;

    // percent
    int m_percent_margin = 15;
    int m_percent_height = 80;

    // foot
    float m_foot_margin = 3;

    // calc
    const float m_minilize_button_x_l = m_fix_width - m_btn_margin * 2 - m_btn_size * 2;
    const float m_minilize_button_x_r = m_minilize_button_x_l + m_btn_size;
    const float m_minilize_button_y_t = m_btn_margin;
    const float m_minilize_button_y_b = m_btn_margin_top + m_btn_size;

    const float m_close_button_x_l = m_fix_width - m_btn_margin - m_btn_size;
    const float m_close_button_x_r = m_fix_width - m_btn_margin;
    const float m_close_button_y_t = m_btn_margin;
    const float m_close_button_y_b = m_btn_margin_top + m_btn_size;

    const float m_foot_progress_back_y = m_fix_height - m_foot_margin;

    const float m_text_area_x = m_btn_margin * 4;

    const float m_icon_area_y = (m_fix_height - m_icon_size) / 2;

    const float m_file_name_x = m_icon_margin + m_file_name_margin + m_icon_size;
    const float m_file_name_y = m_fix_height / 2 - m_file_name_height / 2;
    const float m_file_name_w = m_fix_width - m_icon_size - m_icon_margin - m_pause_btn_height * 2 - m_file_name_margin * 3;

    const float m_progress_pause_x = m_file_name_x + m_file_name_w + m_pause_btn_height;
    const float m_progress_pause_x_r = m_progress_pause_x + m_pause_btn_height;
    const float m_progress_pause_y = (m_fix_height - m_pause_btn_height) / 2;
    const float m_progress_pause_y_b = (m_fix_height + m_pause_btn_height) / 2;

    const float m_percent_x = 0;
    const float m_percent_y = m_fix_height - m_foot_margin - m_percent_height - m_percent_margin / 5;

    // progress
    bool m_show = false;
    bool m_pause = false;
    float m_move_x = 0.5;
    bool m_stopping = false;
    float m_current_value = 0.0;
    QString m_file_name = tr("starting ...");
    QIcon m_icon = QIcon::fromTheme("text");
};

class OtherButton : public QWidget
{
    Q_OBJECT
public:
    explicit OtherButton(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

Q_SIGNALS:
    void clicked (bool show);

private:
    // button height
    int m_icon_margin = 6;
    int m_icon_size = 10;
    int m_button_heigth = 24;

    // text width
    int m_text_length = 120;

    // show
    bool m_show = false;

    bool m_is_press = true;
};

#endif // FILEOPERATIONPROGRESS_H
