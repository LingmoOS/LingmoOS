#!/bin/bash

aiAgentConf=/etc/dde-ai-agent.conf

if [ -f $aiAgentConf  ]; then
  aiAgent=$(head -1 "$aiAgentConf")
  if [ -x $aiAgent ]; then
    exec $aiAgent
  fi
fi

if [ -x /usr/bin/uos-ai-assistant ]; then
  /usr/bin/uos-ai-assistant --talk
else
  exec /usr/libexec/dde-daemon/keybinding/shortcut-dde-grand-search.sh
fi
