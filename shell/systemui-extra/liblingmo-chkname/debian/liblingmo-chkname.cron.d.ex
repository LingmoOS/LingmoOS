#
# Regular cron jobs for the liblingmo-chkname package
#
0 4	* * *	root	[ -x /usr/bin/liblingmo-chkname_maintenance ] && /usr/bin/liblingmo-chkname_maintenance
