/*
 * 1:计算是否闰年
 * 2:计算国际节日
 * 3:计算二十四节气
 * 4:计算农历年 天干+地支+生肖
 * 5:计算指定年月日农历信息,包括公历节日和农历节日及二十四节气
 */
#ifndef CALENDARINFO_H
#define CALENDARINFO_H

#include <QObject>

#define year_2099
class CalendarInfo : public QObject
{
    Q_OBJECT
public:
    explicit CalendarInfo(QObject *parent = nullptr);

    static CalendarInfo &getInstance()
    {
        static CalendarInfo m_pInstance;
        return m_pInstance;
    }

    QString m_LunarYear;
    QString m_LunarMonth;
    QString m_LunarDay;
    //计算是否闰年
    bool isLoopYear(int year);

    //计算指定年月该月共多少天
    int getMonthDays(int year, int month);

    //计算指定年月对应到该月共多少天
    int getTotalMonthDays(int year, int month);

    //计算指定年月对应星期几
    int getFirstDayOfWeek(int year, int month, bool FirstDayisSun);

    //计算国际节日
    QString getHoliday(int month, int day);

    //计算二十四节气
    QString getSolarTerms(int year, int month, int day);

    //计算农历节日(必须传入农历年份月份)
    QString getLunarFestival(int month, int day);

    //计算农历年 天干+地支+生肖
    QString getLunarYear(int year);

    //计算指定年月日农历信息,包括公历节日和农历节日及二十四节气
    void getLunarCalendarInfo(int year, int month, int day, QString &strHoliday, QString &strSolarTerms,
                              QString &strLunarFestival, QString &strLunarYear, QString &strLunarMonth,
                              QString &strLunarDay);

    //获取指定年月日农历信息
    QString getLunarInfo(int year, int month, int day, bool yearInfo, bool monthInfo, bool dayInfo);
    QString getLunarYearMonthDay(int year, int month, int day);
    QString getLunarMonthDay(int year, int month, int day);
    QString getLunarDay(int year, int month, int day);

private:
    QList<int> m_lunarCalendarTable;    //农历年表
    QList<int> m_springFestival;        //春节公历日期
    QList<int> m_lunarData;             //农历每月数据
    QList<int> m_chineseTwentyFourData; //农历二十四节气数据
    QList<int> m_monthAdd;              //公历每月前面的天数

    QList<QString> m_listDayName;   //农历日期名称集合
    QList<QString> m_listMonthName; //农历月份名称集合
    QList<QString> m_listSolarTerm; //二十四节气名称集合

    QList<QString> m_listTianGan;  //天干名称集合
    QList<QString> m_listDiZhi;    //地支名称集合
    QList<QString> m_listShuXiang; //属相名称集合

    void initLunarCalendarTable();    //初始化农历年表
    void initSpringFestival();        //初始化春节公历日期
    void initLunarData();             //初始化农历每月数据
    void initChineseTwentyFourData(); //初始化农历二十四节气数据
    void initMonthAdd();              //初始化公历每月前面的天数
    void initOther();                 //其他

signals:
};

#endif // CALENDARINFO_H
