# SPDX-License-Identifier: GPL-3.0-or-later
import ast
import configparser
from pathlib import Path
import runpy
import tempfile
from types import SimpleNamespace
import unittest
import yaml

SOURCE = Path(__file__).resolve().parents[1]
final = runpy.run_path(str(SOURCE / "helpers/calamares-sources-final"))
media = runpy.run_path(str(SOURCE / "helpers/calamares-sources-media"))

class InstalledSystem(unittest.TestCase):
    def test_removes_local_sources_and_live_marker_preserves_remote(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp); sources=root/"etc/apt/sources.list.d"; sources.mkdir(parents=True)
            (root/"etc/debian_chroot").write_text("live\n")
            (sources/"live.list").write_text("deb [trusted=yes] file:/run/live/medium trixie main\n")
            (sources/"mixed.list").write_text("deb cdrom:[Lingmo]/ trixie main\ndeb https://repo.example/ stable main\n")
            (sources/"mixed.sources").write_text("Types: deb\nURIs: file:///run/live/medium\n https://repo.example/debian\nSuites: trixie\nComponents: main\nSigned-By: /usr/share/keyrings/test.gpg\n\nTypes: deb\nURIs: cdrom:test\nSuites: trixie\nComponents: main\n")
            (sources/"lingmo.list").write_text("deb https://download.opensuse.org/repositories/home:/LingmoOS/Debian_13/ /\n")
            startup=root/"etc/xdg/autostart";startup.mkdir(parents=True);(startup/"lingmo-live-installer.desktop").write_text("live only")
            final["finalize"](root)
            self.assertFalse((root/"etc/debian_chroot").exists())
            self.assertFalse((sources/"live.list").exists())
            self.assertFalse((startup/"lingmo-live-installer.desktop").exists())
            self.assertIn("repo.example",(sources/"mixed.list").read_text())
            mixed=(sources/"mixed.sources").read_text();self.assertNotIn("file:",mixed);self.assertNotIn("cdrom:",mixed);self.assertIn("Signed-By:",mixed)
            self.assertIn("LingmoOS",(sources/"lingmo.list").read_text())
            before={p.name:p.read_text() for p in sources.iterdir()};final["finalize"](root)
            self.assertEqual(before,{p.name:p.read_text() for p in sources.iterdir()})

    def test_refuses_host_and_escape(self):
        with self.assertRaises(ValueError):final["finalize"]("/")
        with tempfile.TemporaryDirectory() as tmp, tempfile.TemporaryDirectory() as outside:
            root=Path(tmp);(root/"etc").mkdir();(root/"etc/apt").symlink_to(outside)
            with self.assertRaises(ValueError):final["finalize"](root)
            self.assertEqual(list(Path(outside).iterdir()),[])

    def test_media_cleanup_does_not_need_network(self):
        with tempfile.TemporaryDirectory() as tmp:
            root=Path(tmp);directory=root/"etc/apt/sources.list.d";directory.mkdir(parents=True)
            (directory/"LINGMO_INSTALLER.list").write_text("deb file:/run/live/medium trixie main")
            calls=[];media["configure"](root,True,lambda *args,**kw:calls.append(args))
            self.assertFalse((directory/"LINGMO_INSTALLER.list").exists());self.assertEqual(calls,[])

    def test_password_and_service_configuration(self):
        users=yaml.safe_load((SOURCE/"calamares/modules/users.conf").read_text())
        self.assertEqual(users["passwordRequirements"],{"minLength":1})
        self.assertFalse(users["allowWeakPasswords"])
        units=yaml.safe_load((SOURCE/"calamares/modules/services-systemd.conf").read_text())["units"]
        self.assertFalse(any(u["name"]=="firstboot.service" for u in units))
        seq=yaml.safe_load((SOURCE/"calamares/settings.conf").read_text())["sequence"][1]["exec"]
        self.assertLess(seq.index("sources-media-unmount"),seq.index("sources-final"));self.assertLess(seq.index("sources-final"),seq.index("umount"))

    def test_installed_desktop_without_oobe(self):
        installer=SOURCE.parent/"lingmo-installer/src/modules/displaymanager/main.py"
        tree=ast.parse(installer.read_text());klass=next(n for n in tree.body if isinstance(n,ast.ClassDef) and n.name=="DMlingmodm")
        method=next(n for n in klass.body if isinstance(n,ast.FunctionDef) and n.name=="set_autologin")
        for auto in (False,True):
            with self.subTest(autologin=auto),tempfile.TemporaryDirectory() as tmp:
                root=Path(tmp);(root/"etc").mkdir();(root/"etc/lingmo-dm.conf").write_text("[Theme]\nName=existing\n[Autologin]\nUser=live\nSession=lingmo-oobe\n")
                ns={"os":__import__("os"),"libcalamares":SimpleNamespace(globalstorage=SimpleNamespace(value=lambda key:"alice"))}
                exec(compile(ast.Module(body=[method],type_ignores=[]),str(installer),"exec"),ns)
                ns["set_autologin"](SimpleNamespace(root_mount_point=tmp),"alice" if auto else None,auto,None)
                config=configparser.ConfigParser();config.read(root/"etc/lingmo-dm.conf")
                self.assertEqual(config["Autologin"]["Session"],"lingmo-wayland")
                self.assertEqual(config["Autologin"]["User"],"alice" if auto else "")
                self.assertEqual(config["Theme"]["Name"],"existing")
                self.assertFalse((root/"etc/lingmo-oobe-account.conf").exists())

if __name__ == "__main__":unittest.main()
