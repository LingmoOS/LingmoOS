#
# Regular cron jobs for the libdareader package
#
0 4	* * *	root	[ -x /usr/bin/libdareader_maintenance ] && /usr/bin/libdareader_maintenance
