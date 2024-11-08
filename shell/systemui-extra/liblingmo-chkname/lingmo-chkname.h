#ifndef LINGMO_USERNAME_CHECK_H
#define LINGMO_USERNAME_CHECK_H

#ifdef __cplusplus
extern "C" {
#endif

#define CHECK_SUCCESS        0
#define LENGTH_ERROR        -1
#define REGEX_ERROR         -2
#define RESERVED_ERROR      -3
#define NAME_ERROR          -4
#define OPEN_RESERVED_NAMES_ERROR          -5
#define FIRST_CHAR_ERROR    -6
#define HEX_ERROR           -7
#define OCTAL_ERROR         -8
#define NUMBERIC_ERROR      -9


/* 
 * @brief  检测用户名是否合法
 *
 * @param name 待检测用户名
 * @param reserve 是否过滤系统保留用户名
 * 1表示过滤（保留用户名为非法用户名），0表示不过滤
 * 
 * @return 用户名合法返回0，用户名非法返回负数
 */
int lingmo_username_check(const char *name, int reserve);


/*
 * @brief 将返回值转为字符串描述
 * 
 * @param err_num lingmo_username_check接口返回值
 * 
 * @return 参数err_num为已知错误时，返回对应的字符串描述；
 * 未知错误，返回“Unknown error”
 */
char *lingmo_username_strerror(int err_num);

#ifdef __cplusplus
}
#endif

#endif /* LINGMO_USERNAME_CHECK_H */
