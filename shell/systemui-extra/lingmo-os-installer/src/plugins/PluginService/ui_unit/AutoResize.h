#ifndef AUTORESIZE_H
#define AUTORESIZE_H
#include <QWidget>
#include <QRadioButton>
#include <QMap>
struct AutoResizeOriginalData
{
	QRect data_rect;
	QFont data_font;
};
class AutoResize
{
public:
	typedef enum
	{
		INCLUDE_LABEL=0x1,
		INCLUDE_BUTTON=0x2,
		INCLUDE_EDITOR=0x4,
        INCLUDE_COMBOBOX=0x8,
        INCLUDE_TEXTBROWER


	}AUTORESIZEFLAG;
	AutoResize(QWidget* obj,float base_width,float base_height);
	~AutoResize(void);
	void doAutoResize();
	void addOtherItem(QWidget *other);
	void addIgnoreItem(QWidget *ignore);	
	void setAutoResizeFlag(int flag);
	void pushAllResizeItem();
    float getHorResizeRatio();
    float getVerResizeRatio();
    float getFontResizeRatio();
private:
	bool m_autoResize;
	QMap<QWidget*,AutoResizeOriginalData> m_resizeMap;
	QMap<QWidget*,AutoResizeOriginalData> m_fontMap;
	QList<QWidget*> m_otherItem;
	QList<QWidget*> m_ignoreItem;
	int m_autoResizeFlag;
	float m_horRatio;
	float m_verRatio;
	float m_fontRatio;
	float m_baseWidth;
	float m_baseHeight;
	QWidget *m_autoResizeObj;
	void calculateResizeRatio();
   void fontAutoResize(QWidget *obj,int fontSIze);
   void dealLayoutItems();
   void ignoreAllChiledren(QObject* obj);
};
#endif//AUTORESIZE_H

