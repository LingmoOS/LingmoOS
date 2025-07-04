#!/usr/bin/python3

'''Apport package hook for sudo

(c) 2010 Canonical Ltd.
Contributors:
Marc Deslauriers <marc.deslauriers@canonical.com>

This program is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 2 of the License, or (at your
option) any later version.  See http://www.gnu.org/copyleft/gpl.html for
the full text of the license.
'''

from apport.hookutils import *

def add_info(report, ui):

    response = ui.yesno("The contents of your /etc/sudoers file may help developers diagnose your bug more quickly, however, it may contain sensitive information.  Do you want to include it in your bug report?")

    if response == None: #user cancelled
        raise StopIteration

    elif response == True:
        # This needs to be run as root
        report['Sudoers'] = root_command_output(['/bin/cat', '/etc/sudoers'])
        report['VisudoCheck'] = root_command_output(['/usr/sbin/visudo', '-c'])

    elif response == False:
        ui.information("The contents of your /etc/sudoers will NOT be included in the bug report.")


