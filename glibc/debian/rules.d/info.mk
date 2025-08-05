#
# Add here instructions to dump useful debug information
#

$(stamp)info:
	@echo '------------------------------------------------------'
	
	uname -a
	@echo

	if [ -f /proc/cpuinfo ] ; then cat /proc/cpuinfo ; fi
	@echo

	@echo '------------------------------------------------------'

	touch $@
