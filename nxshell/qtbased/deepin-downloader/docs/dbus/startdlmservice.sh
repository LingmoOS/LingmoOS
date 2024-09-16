#!/bin/bash

gdbus call --session  --dest com.deepin.SessionManager --object-path /com/deepin/StartManager --method com.deepin.StartManager.RunCommand "dlmextensionservice" "['&']"
