#ifndef CALENDARDATABASE_H
#define CALENDARDATABASE_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QTextCodec>
#include <QSqlError>
#include <QMessageBox>
#include <QDateTime>
#include <QDebug>
#include "schedulewidget.h"
#include "schedulestruct.h"
//#define CSchduleNew
const QString ITEM_FIELD = "id,start_day,start_month,start_hour,start_minute,"
                           "end_hour,end_minute,end_day,end_month,descript,start_date,end_date,"
                           "remind,repeat,beginrepeat,endrepeat,beginDateTime,endDateTime,"
                           "isAllDay,isLunar,allDayremind"; //数据库字段
#define DATABASE_INSTANCE (CalendarDataBase::getInstance())
enum DB_RETURN_STATUS            //数据库操作结果返回表
{
    DB_OP_SUCC = (0),            //数据库操作成功
    DB_UNCONNECT = (-1),         //数据库连接失败导致的错误
    INVALID_INPUT = (-2),        //无效的输入值
    DB_DISORDERD = (-3),         //数据库与外界顺序不符导致的失败
    DB_EMPTY_TABLE = (-4),       //数据库中的表格内容为空
    DB_OP_ADD_FAILED = (-5),     //数据库添加操作失败
    DB_OP_DEL_FAILED = (-6),     //数据库删除操作失败
    DB_OP_GET_FAILED = (-7),     //数据库查询操作失败
    DB_OP_ADD_REPEAT = (-8),     //数据库添加失败，重复添加
    DB_INSIDE_FAILED = (-9),     //数据库内部错误
    SCHEDULE_UNFOUND = (-10),    //数据库中标记未找到
    CREATE_TABLE_FAILED = (-11), //数据库创表失败
    DEL_TABLE_FAILED = (-12),    //数据库删表失败
    DB_OP_ALTER_FAILED = (-13),  //数据库修改操作失败
    OUT_OF_RESULT,               //结果边界
};

#define DATABASE_INSTANCE (CalendarDataBase::getInstance())

class CalendarDataBase : public QObject
{
    Q_OBJECT
public:
    explicit CalendarDataBase(QObject *parent = nullptr);
    ~CalendarDataBase();
    QString userName ;
    QString xdg_runtime_dir;
    QString display;
    QString XorWayland;
    QSqlDatabase m_dataBase;
    static CalendarDataBase &getInstance()
    {
        static CalendarDataBase m_pInstance;
        return m_pInstance;
    }
    int initDataBase();
    int RRuleType;
    int AlarmType;
    void deleteschedule();
    void processExpiredEvents(const QString& expiredIds);
    QStringList getCurrentCrontab();
    void updateCrontabFromFile(const QString& filename);
    //    CSchceduleDlg *dlg = new CSchceduleDlg(1,QWidget,false);
    DB_RETURN_STATUS getStatus(); //获取状态
    QStringList getAllCronJobs(); //获取当前数据库里面每一行 然后转换成crontab形式
    QString convertToCronFormat(const QSqlRecord &record); //格式转换
    void setMarkInfo(const MarkInfo &info); //设置信息结构体
    QStringList getMarkIdList();            //获取日程条Id列表
    MarkInfo getMarkInfo();                 //设置信息结构体
    QList<MarkInfo> getMarkInfoList();      //设置日程条信息结构体列表
    QTime intToTime(int hour, int minute); //处理int 转换成时间的函数
    QString handleTimeLong(QTime time1, QTime time2); //计算两个Time的时长 用来显示日程用
    void add(const MarkInfo &info); //增加日程在crontab
    void update(const MarkInfo &info); //更新日程在crontab
    void del(const MarkInfo &info); //删除日程在crontab
    QStringList m_markIdList;
    QList<MarkInfo> m_markInfoList;
    void addShell();
    QList<MarkInfo> getInfoList();
    QStringList getIdList();
private:
    DB_RETURN_STATUS m_dbStatus;
    MarkInfo m_info;


public slots:

    int insertDataSlot(const MarkInfo &info);
    int deleteDataSlot(const MarkInfo &info);
    int searchDataSlot(const QDate &date);
    int updateDataSlot(const MarkInfo &info);
    int deleteDataSlotfromMarkID(QString markID);

signals:

    void insertDataSignal(const MarkInfo &info);
    void deleteDataSignal(const MarkInfo &info);
    void searchDataSignal(const QDate &date);
    void updateDataSignal(const MarkInfo &info);
};

#endif // CALENDARDATABASE_H
