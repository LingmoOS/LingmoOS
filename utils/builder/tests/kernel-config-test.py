#!/usr/bin/env python3
"""Exercise auto/config without network, root, live-build or an ISO build."""
import json
import os
from pathlib import Path
import shutil
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
for arch in ("amd64", "i386", "arm64"):
    with tempfile.TemporaryDirectory(prefix="lingmo-kernel-config-") as directory:
        work = Path(directory)
        shutil.copytree(ROOT / "scripts/lingmo-config/common", work / "lingmo-config/common")
        (work / "lingmo-config/variant-minimal").mkdir()
        (work / "bin").mkdir()
        (work / "live-build/data/debian-cd/trixie").mkdir(parents=True)
        mocks = {
            "wget": "#!/bin/sh\nprintf 'Codename: trixie\\n'\n",
            "lb": "#!/usr/bin/python3\nimport json,sys\nfrom pathlib import Path\nPath('lb-args.json').write_text(json.dumps(sys.argv[1:]))\n",
        }
        for name, content in mocks.items():
            path = work / "bin" / name
            path.write_text(content)
            path.chmod(0o755)
        env = dict(os.environ, PATH=str(work / "bin") + ":" + os.environ["PATH"],
                   LIVE_BUILD=str(work / "live-build"))
        subprocess.run(["bash", str(ROOT / "scripts/auto/config"), "--architectures", arch,
                        "--variant", "minimal"], cwd=work, env=env, check=True, stdout=subprocess.DEVNULL)
        args = json.loads((work / "lb-args.json").read_text())
        if arch == "amd64":
            assert args[args.index("--linux-flavours") + 1] == "6.18.62-lingmo"
        elif arch == "i386":
            assert args[args.index("--linux-flavours") + 1] == "686-pae"
        else:
            assert "--linux-flavours" not in args
        selected = []
        for path in (work / "config/package-lists").glob("*.list.chroot"):
            enabled = True
            for line in path.read_text().splitlines():
                if line.startswith("#if ARCHITECTURES "):
                    enabled = arch in line.split()[2:]
                elif line == "#endif":
                    enabled = True
                elif enabled and line and not line.startswith("#"):
                    selected.append(line)
        if arch == "amd64":
            for package in ("linux-image-6.18.62-lingmo", "linux-headers-6.18.62-lingmo"):
                assert package in selected, package
            assert "linux-image-amd64" not in selected
            assert "linux-headers-amd64" not in selected
        else:
            assert "linux-image-6.18.62-lingmo" not in selected
            assert "linux-headers-6.18.62-lingmo" not in selected
        for stage in ("chroot", "binary"):
            source = (work / f"config/archives/lingmo.list.{stage}").read_text()
            assert "deb [arch=amd64] https://r2packages.lingmo.org stable main" in source
        print(f"{arch}: kernel config passed")
