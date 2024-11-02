//#include <stdio.h>
//#include <unistd.h>
//#include <sys/stat.h>
//#include <string.h>
//#include <stdlib.h>
//#include <fcntl.h>
//#include <errno.h>
//#include <signal.h>

//#include "main_window.h"
//#include "single.h"

//#define LOCK_FILE_PATH "/tmp/.lingmo-uninstaller"
//#define UNINSTALL_LOCK_FILE_NAME "uninstall.lock"
//#define EXTRA_LOCK_FILE_PATH "/tmp/lock/"
//#define EXTRA_LOCK_FILE_NAME "lingmo-update.lock"

//extern char *p_pkg_name;
//extern char *p_desktop_path;
//extern main_window *w;

///*
// * flag : 0 -> doing not have O_TRUNC flag
// * flag : 1 -> having O_TRUNC falg
// */
//static int get_fd(char *p_file_path_in , int i_flag_in)
//{
//    int i_ret = -1;
//    int fd = -1;

//    if (p_file_path_in == NULL) {
//        printf("arg is NULL\n");
//        return -1;
//    }

//    printf("Info : get fd file path is [%s]\n" , p_file_path_in);

//    i_ret = access(LOCK_FILE_PATH , F_OK);
//    if (i_ret == -1) {
//        printf("Info : dir /tmp/.lingmo-uninstaller is not exits\n");
//        printf("Info : crate dir ......\n");
//        i_ret = mkdir(LOCK_FILE_PATH , 0777);
//        if(i_ret == -1) {
//            printf("Error :create dir is fail\n");
//            printf("Info : %s , errno = %d , errstr = %s\n" , LOCK_FILE_PATH ,  errno , strerror(errno));
//            return -1;
//        }
//        /* modify own mod */
//        if (chown(LOCK_FILE_PATH , getuid() , getgid())) {
//            printf("Error : modify dir own fail\n");
//            return -1;
//        }
//        if (chmod(LOCK_FILE_PATH , 0777)) {
//            printf("Error : modify dir mod fail\n");
//            return -1;
//        }

//        printf("Info : create dir is success\n");
//    }

//    umask(0000);
//    switch (i_flag_in) {
//    case 0:
//        fd = open(p_file_path_in , O_CREAT |  O_RDWR , 0777);
//        break;
//    case 1:
//        fd = open(p_file_path_in , O_CREAT |  O_RDWR | O_TRUNC , 0777);
//        break;
//    default:
//        printf("arg flag is error\n");
//        return -1;
//    }
//    if (fd == -1) {
//        printf("get file fd fail\n");
//        return -1;
//    }

//    return fd;
//}

///*
// * i_flag_in 1 -> uninstaller single judge
// * i_flag_in 2 -> command single judge
// * return 0 -> do not have process is running
// * return 1 -> have process is running
// * return -1 -> fail
// */

////int judge_single(int i_flag_in)
////{
////    int i_ret = -1;
////    int fd = -1;
////    char p_file_path[1024];

////    memset(p_file_path , 0x00 , sizeof(p_file_path));

////    /*get file path*/
////    if (i_flag_in == 1) {
////        snprintf(p_file_path ,sizeof(p_file_path), "%s/%s" , LOCK_FILE_PATH , UNINSTALL_LOCK_FILE_NAME);
////    } else if (i_flag_in == 2) {
////        snprintf(p_file_path ,sizeof(p_file_path), "%s/%s_%s.lock" , LOCK_FILE_PATH , p_pkg_name , p_desktop_path);
//////        p_file_path[strlen(p_file_path)] = "\0";
////        qDebug() << "12312312---->" << p_file_path;
////    }

////    /*add file lock*/
////    struct flock s_lock;
////    s_lock.l_type = F_WRLCK;
////    s_lock.l_whence = SEEK_SET;
////    s_lock.l_start = 0;
////    s_lock.l_len = 0;
////    s_lock.l_pid = getpid();

////    fd = get_fd(p_file_path , 1);
////    if (fd == -1) {
////        printf("get lock file fd fail\n");
////        return -1;
////    }

////    switch (i_flag_in) {
////    case 1 :
////        i_ret = fcntl(fd , F_SETLKW , &s_lock);
////        break;
////    case 2 :
////        i_ret = fcntl(fd , F_SETLK , &s_lock);
////        if (i_ret == -1) {
////            printf("lock file fail . errno is %s\n" , strerror(errno));
////            return 1;
////        }
////        break;
////    default :
////        printf("arg is error\n");
////        return -1;
////    }

////    qDebug()  << "bbbbbb---->"<< p_file_path;

////    //sprintf(p_file_path + strlen(p_file_path) , ".pid");
////    strncat(p_file_path,".pid",strlen(".pid"));

////    qDebug()  << "ccccc---->"<< p_file_path;

////    i_ret = write_pid(p_file_path);
////    if (i_ret == -1) {
////        printf("write pid file is fail\n");
////        return -1;
////    }

////    return 0;
////}

//int unlock_uninstall_file()
//{
//    int i_ret = -1;
//    int fd = -1;
//    char p_file_path[1024];

//    memset(p_file_path , 0x00 , sizeof(p_file_path));

//    snprintf(p_file_path ,sizeof(p_file_path), "%s/%s" , LOCK_FILE_PATH , UNINSTALL_LOCK_FILE_NAME);

//    struct flock s_lock;
//    s_lock.l_type = F_UNLCK;
//    s_lock.l_whence = SEEK_SET;
//    s_lock.l_start = 0;
//    s_lock.l_len = 0;
//    s_lock.l_pid = getpid();

//    fd = get_fd(p_file_path , 1);
//    if (fd == -1) {
//        printf("get lock file fd fail\n");
//        return -1;
//    }

//    i_ret = fcntl(fd , F_SETLK , &s_lock);
//    if (i_ret == -1) {
//        printf("unlock uninstall file fail\n");
//        return -1;
//    }

//    return 0;
//}

//static int write_pid(char *p_file_path_in)
//{
//    int i_ret = -1;
//    off_t off_ret = -1;
//    int fd = -1;
//    char p_buff[1024];

//    memset(p_buff , 0x00 , sizeof(p_buff));

//    fd = get_fd(p_file_path_in , 1);
//    if(fd == -1) {
//        printf("get pid file fd is fail\n");
//        return -1;
//    }

//    off_ret = lseek(fd , 0 , SEEK_SET);
//    if (off_ret == -1) {
//        printf("lseek pid file head point is fail\n");
//        return -1;
//    }

//    qDebug() << "eee---->"<< p_buff;

//    snprintf(p_buff ,sizeof(p_buff), "pid=%d" , getpid());

//    qDebug() << "ffff---->"<< p_buff;


//    i_ret = write(fd , p_buff , strlen(p_buff));
//    if(i_ret == -1) {
//        printf("write pid file is fail\n");
//        return -1;
//    }
//    close(fd);

//    return 0;
//}

///*
// * i_flag_in : 1 -> uninstall pid
// * i_flag_in : 2 -> command pid
// */
////int read_pid(int i_flag_in)
////{
////    int fd = -1;
////    int i_ret = -1;
////    char *p_tmp = NULL;
////    char p_buff[1024];
////    char p_file_path[1024];

////    memset(p_buff , 0x00 , sizeof(p_buff));
////    memset(p_file_path , 0x00 , sizeof(p_file_path));

////    if (i_flag_in == 1) {
////        snprintf(p_file_path ,sizeof(p_file_path), "%s/%s.pid" , LOCK_FILE_PATH , UNINSTALL_LOCK_FILE_NAME);
////    } else if (i_flag_in == 2) {
////        snprintf(p_file_path ,sizeof(p_file_path), "%s/%s_%s.lock.pid" , LOCK_FILE_PATH , p_pkg_name , p_desktop_path);
////    }

////    fd = get_fd(p_file_path , 0);
////    if (fd == -1) {
////        printf("get pid file fd is fail\n");
////        return -1;
////    }

////    i_ret = read(fd , p_buff , sizeof(p_buff));
////    if (i_ret == -1) {
////        printf("read pid file is fail\n");
////        return -1;
////    }

////    close(fd);

////    p_tmp = strchr(p_buff , '=');
////    if (p_tmp == NULL) {
////        printf("read pid file is not find char '='\n");
////        return -1;
////    }

////    return atoi(p_tmp + 1);
////}

//int catch_signal()
//{
//    int i_ret = -1;
//    struct sigaction s_signal;
//    memset(&s_signal , 0x00 , sizeof(struct sigaction));

//    s_signal.sa_handler = sig_callback_35;
//    sigemptyset(&s_signal.sa_mask);
//    s_signal.sa_flags = 0;

//    i_ret = sigaction(35 , &s_signal , NULL);
//    if(i_ret == -1) {
//        printf("set catch signal is fail . errno is %s\n" , strerror(errno));
//        return -1;
//    }

//    return 0;
//}

//void sig_callback_35(int signal)
//{
//    printf("Info : catch signal 35!!!\n");
//    if (w != NULL) {
//        w->hide();
//        w->show();
//        w->activateWindow();
//    }
//    return;
//}

//int kill_signal(int pid)
//{
//    int i_ret = -1;
//    i_ret = kill(pid , 35);
//    if (i_ret == -1) {
//        printf("kill signal is fail\n");
//        return -1;
//    }
//    return 0;
//}

//void init_export_args(char *p_str_in)
//{
//    if (p_str_in == NULL) {
//        printf("arg is error\n");
//        return;
//    }
//    while (*p_str_in != '\0') {
//        if (*p_str_in == '/') {
//            *p_str_in = '_';
//        }
//        p_str_in++;
//    }
//    return;
//}

//int lock_extra_update_lock(void)
//{
//    int fd = 0;
//    char p_lock_file_path[1024];
//    char p_buf[1024];
//    char p_time[125];
//    struct flock s_lock;

//    memset(&s_lock , 0x00 , sizeof(struct flock));
//    memset(p_lock_file_path , 0x00 , sizeof(p_lock_file_path));
//    memset(p_buf , 0x00 , sizeof(p_buf));
//    memset(p_time , 0x00 , sizeof(p_time));

//    /* judge extar lock file dir whether exits */
//    if (access(EXTRA_LOCK_FILE_PATH , F_OK)) {
//        printf("Info : create extra lock file path...");
//        if (mkdir(EXTRA_LOCK_FILE_PATH , 0777)) {
//        printf("Error : create extra lock file path fail\n");
//        return -1;
//        }
//        printf("Info : create extra lock file path success\n");
//    }

//    /* get local time */
//    time_t off_time;
//    struct tm *s_time = NULL;
//    time(&off_time);
//    s_time = localtime(&off_time);
//    if (s_time == NULL) {
//        printf("get local time fail\n");
//        return -1;
//    }
//    snprintf(p_time ,sizeof(p_time), "%4.4d-%2.2d-%2.2d %2.2d:%2.2d:%2.2d" , s_time->tm_year + 1900 , s_time->tm_mon + 1 , s_time->tm_mday , s_time->tm_hour , s_time->tm_min , s_time->tm_sec);

//    /* lock extra lock file */
//    snprintf(p_lock_file_path ,sizeof(p_lock_file_path), "%s%s" , EXTRA_LOCK_FILE_PATH , EXTRA_LOCK_FILE_NAME);
//    fd = open(p_lock_file_path , O_CREAT | O_RDWR | O_TRUNC , 0666);
//    if (fd == -1) {
//        printf("Error : get extra lock file fd fail\n");
//        return -1;
//    }

//    s_lock.l_type = F_WRLCK;
//    s_lock.l_whence = SEEK_SET;
//    s_lock.l_start = 0;
//    s_lock.l_len = 0;

//    if (fcntl(fd , F_SETLKW , &s_lock) == -1) {
//        printf("Erro : lock extra lock file fail\n");
//        close(fd);
//        return -1;
//    }

//   /* write infomation to extra lock file */
//    snprintf(p_buf ,sizeof(p_buf), "%d\nlingmo-uninstall\nremove\n%s\n" , getuid() , p_time);
//    if (write(fd , p_buf , strlen(p_buf)) == -1) {
//        printf("write infomation to extra lock file fail\n");
//        close(fd);
//        return -1;
//    }
//    return 0;
//}

//int unlock_extra_update_lock(void)
//{
//   char p_lock_file_path[1024];
//   struct flock s_lock;
//   int fd = 0;

//   memset(p_lock_file_path , 0x00 , sizeof(p_lock_file_path));
//   memset(&s_lock , 0x00 , sizeof(struct flock));

//   snprintf(p_lock_file_path ,sizeof(p_lock_file_path), "%s%s" , EXTRA_LOCK_FILE_PATH , EXTRA_LOCK_FILE_NAME);

//   fd = open(p_lock_file_path , O_RDWR);
//   if(fd == -1)
//   {
//      printf("Error : get file fd fail\n");
//      return -1;
//   }

//   s_lock.l_type = F_UNLCK;
//   s_lock.l_whence = SEEK_SET;
//   s_lock.l_start = 0;
//   s_lock.l_len = 0;

//   if(fcntl(fd , F_SETLK , &s_lock) == -1)
//   {
//      printf("Error : unlock extra lock file fail\n");
//      return -1;
//   }

//   close(fd);

//   return 0;
//}
