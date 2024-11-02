/*
 * Copyright (C) 2019 Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
 *
 */


#ifndef __USER_ACCOUNTS_H__
#define __USER_ACCOUNTS_H__

#include<stdbool.h>

#ifdef __cplusplus
extern "C"{
#endif

/*
 * create new user.
 * @username is the login name.
 * @accounttype is the user type, "0" is the standard user, and "1" is the administrators.
 * @password is the login password.
 * @iconfile is the path of the user image.
 */
void lingmo_accounts_user_createuser(char *username,
                       int accounttype,
                       char *password,
                       char *iconfile);

/*
 * set to login automatically.
 * @login can be set to true, indicating automatic login.
 */
void lingmo_accounts_user_autologin(char *username, bool login);

/*
 * change the user image.
 * @iconfile is the path of the user image.
 */
void lingmo_accounts_user_chgusericon(char *username,
                        char *iconfile);

/*
 * change the user password.
 * @password is the login password.
 */
void lingmo_accounts_user_chguserpasswd(char *username,
                          char *password);

/*
 * change the user type.
 * @accounttype is the user type, "0" is the standard user, and "1" is the administrators.
 */
void lingmo_accounts_user_chgusertype(char *username,
                        int accounttype);

/*
 * delete the user.
 * @remove represents whether to delete the user directory.
 */
void lingmo_accounts_user_deleteuser(char *username,
                       bool removefile);

#ifdef __cplusplus
}
#endif

#endif
