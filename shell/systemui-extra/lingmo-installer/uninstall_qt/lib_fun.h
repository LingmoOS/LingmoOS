#ifndef __LIB_FUN_H__
#define __LIB_FUN_H__

#include <QString>
#include <QSettings>

int snow_analysis_xml(char *p_desktop_in , char *p_pkg_name_out , char *p_runtime_out);
QString getAndroidPackageName(QString desktopFilePath);
QString snow_get_local_pkg_name(QString p_desktop_path_in);

QString get_zhname(QString p_desktop_path_in);
void remove_last_line_break(char *p_src_in_out);
QString intercept_chinese_characters(QString src_in);
QString SelectPackgeName(QString p_desktop_in);

#endif
