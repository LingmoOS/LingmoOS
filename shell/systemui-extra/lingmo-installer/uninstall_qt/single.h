#ifndef __SINGLE_H__
#define __SINGLE_H__

static int get_fd(char *p_file_path_in , int i_flag_in);
extern int judge_single(int i_flag_in);
extern int unlock_uninstall_file();
static int write_pid(char *p_file_path_in);
extern int read_pid(int i_flag_in);
extern int catch_signal();
extern void sig_callback_35(int);
extern int kill_signal(int);
extern void init_export_args(char *);
extern int lock_extra_update_lock(void);
extern int unlock_extra_update_lock(void);

#endif
