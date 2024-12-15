#!/bin/bash

gdbus call --session  --dest com.lingmo.SessionManager --object-path /com/lingmo/StartManager --method com.lingmo.StartManager.RunCommand "dlmextensionservice" "['&']"
