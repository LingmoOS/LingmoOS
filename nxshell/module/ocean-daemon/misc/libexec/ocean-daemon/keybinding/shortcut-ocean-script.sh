#!/bin/bash

aiAgentConf=/etc/ocean-ai-agent.conf

if [ -f $aiAgentConf  ]; then
  aiAgent=$(head -1 "$aiAgentConf")
  if [ -x $aiAgent ]; then
    exec $aiAgent
  fi
fi

if [ -x /usr/bin/uos-ai-assistant ]; then
  /usr/bin/uos-ai-assistant --talk
else
  exec /usr/libexec/ocean-daemon/keybinding/shortcut-ocean-grand-search.sh
fi
