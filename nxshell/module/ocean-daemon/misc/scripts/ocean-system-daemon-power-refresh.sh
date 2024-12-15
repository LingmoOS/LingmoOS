#!/bin/sh
case $1/$2 in
        pre/*)
        ;;
        post/*)
            gdbus call -y -d org.lingmo.ocean.Power1 -o /org/lingmo/ocean/Power1 -m org.lingmo.ocean.Power1.Refresh --timeout 2
        ;;
esac
