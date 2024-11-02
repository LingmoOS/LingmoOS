#include "GSettingSubject.h"


GsettingSubject::GsettingSubject(QObject *parent) : QObject(parent)
{
    iniData();
    iniConnection();
}

void GsettingSubject::iniConnection()
{
    if(m_styleSettings!=nullptr){
        connect(m_styleSettings, &QGSettings::changed, this, [=] (const QString &key){
            if(key==STYLE_NAME){
                if(m_stylelist.contains(m_styleSettings->get(STYLE_NAME).toString())){
                    emit blackStyle();
                }else{
                    emit whiteStyle();
                }
            }
            if(key==STYLE_ICON_NAME || key==STYLE_ICON){
               emit iconChnaged();
            }
            if (key == SYSTEM_FONT_SIZE) {
                const int size = m_styleSettings->get(SYSTEM_FONT_EKY).toInt();
                emit fontChanged(size);
            }
        });
    }
}

void GsettingSubject::iniData()
{
    const QByteArray style_id(ORG_LINGMO_STYLE);
    m_stylelist<<STYLE_NAME_KEY_DARK<<STYLE_NAME_KEY_BLACK;
    if(QGSettings::isSchemaInstalled(style_id)){
        m_styleSettings = new QGSettings(style_id);
    }
    const QByteArray timeId(FORMAT_SCHEMA);
    if (QGSettings::isSchemaInstalled(timeId)){
        m_formatSettings = new QGSettings(timeId);
    }
    const QByteArray mouseId(MOUSE_SCHEMA);
    if (QGSettings::isSchemaInstalled(mouseId)){
        m_mouseSettings = new QGSettings(mouseId);
    }
}

/**
 * @brief 初始化主题
 */
void GsettingSubject::iniWidgetStyle()
{
    if(m_styleSettings!=nullptr){
        if(m_stylelist.contains(m_styleSettings->get(STYLE_NAME).toString())){
            emit blackStyle();
        }else{
            emit whiteStyle();
        }
    }

}
/**
 * @brief 初始化时区
 */
void GsettingSubject::iniTimeZone()
{
}
/**
 * @brief 初始化字体
 */
void GsettingSubject::iniFontSize()
{
    if(m_styleSettings!=nullptr){
        //字体
        if (m_styleSettings->get(SYSTEM_FONT_EKY).toInt()) {
            const int size = m_styleSettings->get(SYSTEM_FONT_EKY).toInt();
            emit fontChanged(size);
        }
    }
}



GsettingSubject::~GsettingSubject()
{
    delete m_styleSettings;
    delete m_formatSettings;
    delete m_mouseSettings;
}
