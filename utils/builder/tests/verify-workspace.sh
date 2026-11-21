#!/usr/bin/env bash
set -euo pipefail
workspace=$(cd "$(dirname "$0")/../.." && pwd)
export PYTHONDONTWRITEBYTECODE=1
python3 "$workspace/lingmo-installer-config/tests/test_installed_system.py"
python3 "$workspace/lingmo-builder/tests/test_live_release.py"
python3 "$workspace/lingmo-initial-setup/tests/test_state.py"
bash -n "$workspace/lingmo-builder/tools/publish/sourceforge.sh"
bash -n "$workspace/lingmo-builder/scripts/lingmo-config/common/hooks/live/lingmo-hacks.chroot"
bash -n "$workspace/lingmo-builder/scripts/auto/config"
sh -n "$workspace/lingmo-builder/scripts/lingmo-config/common/includes.chroot_after_packages/usr/bin/lingmo-live-installer"
python3 - "$workspace" <<'PY'
import ast,pathlib,sys,yaml
root=pathlib.Path(sys.argv[1])
ast.parse((root/'lingmo-installer/src/modules/displaymanager/main.py').read_text())
for name in ('sources-final','sources-media','sources-media-unmount'):
    directory=root/'lingmo-installer-config/calamares-modules'/name
    descriptor=yaml.safe_load((directory/'module.desc').read_text())
    assert descriptor['interface']=='python' and descriptor['script']=='main.py'
    ast.parse((directory/'main.py').read_text())
print('Installer module descriptors and Python/shell syntax passed')
PY
