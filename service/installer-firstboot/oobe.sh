#!/bin/bash
# Lingmo OS OOBE launcher
#   oobe.sh setup   – pre-DM: write SDDM first-boot autologin config
#   oobe.sh         – session: launch lingmo-oobe

case "${1:-run}" in
    setup)
        mkdir -p /etc/sddm.conf.d
        cat > /etc/sddm.conf.d/lingmo-firstboot.conf << 'EOF'
[Autologin]
User=liveuser
Session=lingmo-oobe.desktop
EOF
        ;;
    run|*)
        exec /usr/bin/lingmo-oobe
        ;;
esac
