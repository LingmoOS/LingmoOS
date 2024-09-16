#!/bin/sh
case $1/$2 in
        pre/*)
        ;;
        post/*)
            gdbus call -y -d org.deepin.dde.Power1 -o /org/deepin/dde/Power1 -m org.deepin.dde.Power1.Refresh --timeout 2
        ;;
esac
