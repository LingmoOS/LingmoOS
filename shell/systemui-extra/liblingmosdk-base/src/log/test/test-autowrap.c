/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yunhe Liu <liuyunhe@kylinos.cn>
 *
 */


#include <libkylog.h>
#include <assert.h>

int main(int argc , char** argv)
{
	assert(kdk_logger_init("./logtest.conf") == 0);
    kdk_logger_set_autowrap(1);
	klog_calltrace();
	klog_debug("Debug");
	klog_info("Info");
	klog_notice("Notice");
	klog_warning("Warning");
	klog_err("Error");
	klog_crit("Crit");
	klog_alert("Alert");
	klog_emerg("Emerg");

	return 0;
}
