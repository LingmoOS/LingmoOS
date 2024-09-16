## 获取农历信息

| DBus | `com.deepin.dataserver.Calendar`                                             |
| ---- | ---------------------------------------------------------------------------- |
| 路径 | `/com/deepin/dataserver/Calendar/HuangLi`                                    |
| 接口 | `com.deepin.dataserver.Calendar.HuangLi`                                     |
| 方法 | `getHuangLiMonth (UInt32 year, UInt32 month, Boolean fill) ↦ (String arg_0)` |

参数说明：

- year 公历年
- month 公历月
- fill 是否从周日开始补齐上下月信息。例如 2024 年 3 月 1 号是星期五（农历二十一），如果 fill 为 false，返回的数据从星期五（农历二十一）开始，如果 fill 为 true，返回的数据从星期日（农历十六）开始。

接口返回的是一个 json 对象数组序列化后的字符串，部分字段已弃用，主要关注 GanZhiDay、GanZhiMonth、GanZhiYear、LunarDayName、LunarMonthName、Zodiac、Term

| 字段           | 解释                                                                   |
| -------------- | ---------------------------------------------------------------------- |
| Avoid          | 用于指定应该避免的事项或活动。                                         |
| GanZhiDay      | 表示农历日期的干支表示法，"己未"指的是具体的天干和地支的组合。         |
| GanZhiMonth    | 表示农历月份的干支表示法，"丙寅"指的是具体的天干和地支的组合。         |
| GanZhiYear     | 表示农历年份的干支表示法，"甲辰"指的是具体的天干和地支的组合。         |
| LunarDayName   | 表示农历日期的名称，"十六"表示这一天是农历月份中的第十六天。           |
| LunarFestival  | 用于指定农历的传统节日。                                               |
| LunarLeapMonth | 表示农历是否有闰月，0 表示没有闰月，正常的农历月份。                   |
| LunarMonthName | 表示农历月份的名称，"正月"表示这一天所在的月份是农历的正月。           |
| SolarFestival  | 用于指定阳历的传统节日。                                               |
| Suit           | 用于指定适合进行的活动或事项。                                         |
| Term           | 表示二十四节气中的具体术语。                                           |
| Worktime       | 表示工作时间，但在给定的 JSON 中为 0，可能表示没有特定的工作时间要求。 |
| Zodiac         | 表示生肖，"龙"代表相应的生肖。                                         |

## 获取节假日信息

| DBus | `com.deepin.dataserver.Calendar`                                |
| ---- | --------------------------------------------------------------- |
| 路径 | `/com/deepin/dataserver/Calendar/HuangLi`                       |
| 接口 | `com.deepin.dataserver.Calendar.HuangLi`                        |
| 方法 | `getFestivalMonth (UInt32 year, UInt32 month) ↦ (String arg_0)` |

参数说明：

- year 公历年
- month 公历月

接口返回的是一个 json 对象数组序列化后的字符串，部分字段已弃用，主要关注 list 字段

date：公历日期，例子：2024-05-01
name：节假日名称，例子：劳动节
status：调休标识，1 为休息，2 为调休

## 获取控制中心时间格式

| DTK Config | `Dtk::Core::DConfig`       |
| ---------- | -------------------------- |
| 名字       | `org.deepin.region-format` |
| 配置项     | `shortTimeFormat`          |

## 获取控制中心每周首日

| DTK Config | `Dtk::Core::DConfig`       |
| ---------- | -------------------------- |
| 名字       | `org.deepin.region-format` |
| 配置项     | `firstDayOfWeek`           |
