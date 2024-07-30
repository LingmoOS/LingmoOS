#!/bin/sh
# This helper script unlocks the Plasma5 screenlocker of the session given by argument,
# should the locker ever crash.
#
# Requirements:	ConsoleKit[2] & qdbus
#

# list running sesions
list_sessions ()
{
    ${cklistsessions_EXECUTABLE} | grep -oE '^Session[^:]*'
}

# unlock_session <session_name>
unlock_session ()
{
    ${qdbus_EXECUTABLE} --system org.freedesktop.ConsoleKit "/org/freedesktop/ConsoleKit/$1" org.freedesktop.ConsoleKit.Session.Unlock
}

if [ $# -ne 1 ] ; then
  list_sessions
else
  unlock_session $1
fi

