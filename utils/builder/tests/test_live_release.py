# SPDX-License-Identifier: GPL-3.0-or-later
import json
import os
from pathlib import Path
import subprocess
import tempfile
import unittest
import yaml

SOURCE=Path(__file__).resolve().parents[1]
UPLOAD=SOURCE/"tools/publish/sourceforge.sh"
LIVE=SOURCE/"scripts/lingmo-config/common/includes.chroot_after_packages/usr/bin/lingmo-live-installer"

class LiveRelease(unittest.TestCase):
    def test_oobe_is_not_removed_by_the_live_hook(self):
        hook = (SOURCE/"scripts/lingmo-config/common/hooks/live/lingmo-hacks.chroot").read_text()
        self.assertNotIn("apt-get purge -y lingmo-initial-setup", hook)

    def test_live_user_and_launcher_names_agree(self):
        config=(SOURCE/"scripts/auto/config").read_text()
        self.assertIn("username=lingmo",config)
        hook=(SOURCE/"scripts/lingmo-config/common/hooks/live/lingmo-hacks.chroot").read_text()
        self.assertIn("User=lingmo",hook);self.assertNotIn("User=user\n",hook)
        old=(SOURCE/"scripts/lingmo-config/common/includes.chroot_after_packages/usr/lib/live/config/1165-create-installer-shortcut").read_text()
        self.assertNotIn("cp /usr/share/applications/install-lingmo.desktop",old)

    def test_live_only_autostart(self):
        with tempfile.TemporaryDirectory() as tmp:
            root=Path(tmp);cmdline=root/"cmdline";installer=root/"installer";log=root/"started"
            installer.write_text("#!/bin/sh\nprintf started >> '"+str(log)+"'\n");installer.chmod(0o755)
            script=root/"start";script.write_text(LIVE.read_text().replace("/proc/cmdline",str(cmdline)).replace("/usr/bin/lingmo-installer-config",str(installer)))
            env=dict(os.environ,XDG_RUNTIME_DIR=tmp)
            cmdline.write_text("root=UUID=test quiet")
            subprocess.run(["sh",str(script)],env=env,check=True);self.assertFalse(log.exists())
            cmdline.write_text("boot=live components username=lingmo quiet")
            subprocess.run(["sh",str(script)],env=env,check=True);self.assertEqual(log.read_text(),"started")

    def upload_fixture(self,root):
        bin=root/"bin";bin.mkdir();log=root/"calls.jsonl";images=root/"artifact";images.mkdir();(images/"Lingmo test.iso").write_bytes(b"iso fixture")
        rsync=bin/"rsync";rsync.write_text("#!/usr/bin/python3\nimport json,os,sys\nwith open(os.environ['CALL_LOG'],'a') as f:f.write(json.dumps(sys.argv[1:])+'\\n')\nassert os.path.isfile(os.environ['SF_UPLOAD_KEY'])\nassert os.path.isfile(os.environ['SF_UPLOAD_HOSTS'])\nif '--dry-run' in sys.argv and os.environ.get('MOCK_REMOTE_DIFFERENT'): print('>f+++++++++ missing.iso')\nif '--dry-run' not in sys.argv and os.environ.get('MOCK_DISCARD'): print('Unable to create partial-dir -- discarding completed file.',file=sys.stderr)\nsys.exit(int(os.environ.get('MOCK_RSYNC_EXIT','0')))\n")
        rsync.chmod(0o755)
        sftp=bin/"sftp"
        sftp.write_text("#!/usr/bin/python3\nimport json,os,sys,pathlib\nargs=sys.argv[1:]\nbatch=pathlib.Path(args[args.index('-b')+1]).read_text()\npathlib.Path(os.environ['SFTP_LOG']).write_text(json.dumps({'args':args,'batch':batch}))\nsys.exit(int(os.environ.get('MOCK_SFTP_EXIT','0')))\n")
        sftp.chmod(0o755)
        env=dict(os.environ,PATH=str(bin)+":"+os.environ["PATH"],CALL_LOG=str(log),SFTP_LOG=str(root/"sftp.json"),SF_USERNAME="tester",SF_PROJECT="test-project",SF_REMOTE_DIR="nightly",SF_SSH_PRIVATE_KEY="fixture-key",SF_KNOWN_HOSTS="fixture-host")
        return images,log,env

    def test_upload_complete_iso_then_checksum(self):
        with tempfile.TemporaryDirectory() as tmp:
            root=Path(tmp);images,log,env=self.upload_fixture(root)
            result=subprocess.run(["bash",str(UPLOAD),str(images)],env=env,text=True,capture_output=True)
            self.assertEqual(result.returncode,0,result.stderr)
            calls=[json.loads(line) for line in log.read_text().splitlines()]
            self.assertEqual(len(calls),4);self.assertIn(str(images/"Lingmo test.iso"),calls[0]);self.assertIn(str(images/"sha256sum.txt"),calls[2])
            for tool in ("sha256sum","sha512sum","md5sum"):
                self.assertIn(str(images/(tool+".txt")),calls[2]);self.assertIn(str(images/(tool+".README")),calls[2])
            self.assertNotIn("--delay-updates",calls[0]);self.assertFalse(any(arg.startswith("--partial") for arg in calls[0]));self.assertIn("--dry-run",calls[1]);self.assertIn("--checksum",calls[1]);self.assertTrue(calls[0][-1].endswith("/home/frs/project/test-project/nightly/"))
            wrapper=Path(calls[0][calls[0].index("-e")+1]);self.assertFalse(wrapper.parent.exists())
            self.assertNotIn("fixture-key",result.stdout+result.stderr)

    def test_manifests_verify_and_detect_modified_iso(self):
        import hashlib
        with tempfile.TemporaryDirectory() as tmp:
            images=Path(tmp);image=images/"Lingmo test.iso";image.write_bytes(b"test ISO")
            result=subprocess.run(["bash",str(SOURCE/"tools/publish/checksums.sh"),str(images)],capture_output=True)
            self.assertEqual(result.returncode,0,result.stderr)
            for tool,algorithm in (("sha256sum","sha256"),("sha512sum","sha512"),("md5sum","md5")):
                expected=getattr(hashlib,algorithm)(image.read_bytes()).hexdigest()
                self.assertIn(expected,(images/(tool+".txt")).read_text())
                self.assertNotIn(str(images),(images/(tool+".txt")).read_text())
                self.assertIn("Get-FileHash",(images/(tool+".README")).read_text())
                self.assertEqual(subprocess.run([tool,"--check",tool+".txt"],cwd=images,capture_output=True).returncode,0)
            image.write_bytes(b"corrupted")
            self.assertNotEqual(subprocess.run(["sha256sum","--check","sha256sum.txt"],cwd=images,capture_output=True).returncode,0)

    def test_canonical_version_directory(self):
        with tempfile.TemporaryDirectory() as tmp:
            images,log,env=self.upload_fixture(Path(tmp))
            env["SF_REMOTE_DIR"]="release/iso/lithium";env["SF_BUILD_VERSION"]="26b03"
            result=subprocess.run(["bash",str(UPLOAD),str(images)],env=env,capture_output=True)
            self.assertEqual(result.returncode,0,result.stderr)
            calls=[json.loads(line) for line in log.read_text().splitlines()]
            self.assertTrue(calls[0][-1].endswith("/release/iso/lithium/26b03/"))
            self.assertNotIn("--mkpath",calls[0])
            batch=json.loads((Path(tmp)/"sftp.json").read_text())["batch"]
            for suffix in ("release","release/iso","release/iso/lithium","release/iso/lithium/26b03"):
                path="/home/frs/project/test-project/"+suffix
                self.assertIn('-mkdir "'+path+'"\ncd "'+path+'"',batch)

    def test_zero_exit_discard_is_failure(self):
        with tempfile.TemporaryDirectory() as tmp:
            images,log,env=self.upload_fixture(Path(tmp));env["MOCK_DISCARD"]="1"
            result=subprocess.run(["bash",str(UPLOAD),str(images)],env=env,capture_output=True,text=True)
            self.assertNotEqual(result.returncode,0);self.assertNotIn("Published",result.stdout)
            self.assertEqual(len(log.read_text().splitlines()),1)

    def test_remote_checksum_difference_stops_metadata_upload(self):
        with tempfile.TemporaryDirectory() as tmp:
            images,log,env=self.upload_fixture(Path(tmp));env["MOCK_REMOTE_DIFFERENT"]="1"
            result=subprocess.run(["bash",str(UPLOAD),str(images)],env=env,capture_output=True,text=True)
            self.assertNotEqual(result.returncode,0);self.assertNotIn("Published",result.stdout)
            self.assertEqual(len(log.read_text().splitlines()),2)

    def test_sftp_failure_stops_before_upload(self):
        with tempfile.TemporaryDirectory() as tmp:
            images,log,env=self.upload_fixture(Path(tmp));env["MOCK_SFTP_EXIT"]="1"
            result=subprocess.run(["bash",str(UPLOAD),str(images)],env=env,capture_output=True)
            self.assertNotEqual(result.returncode,0);self.assertFalse(log.exists())
            self.assertTrue((Path(tmp)/"sftp.json").exists())

    def test_version_comes_from_lingmo_version_json(self):
        import runpy
        read=runpy.run_path(str(SOURCE/"tools/publish/version-directory.py"))["build_version"]
        self.assertEqual(read({"source":{"build_ver":"26b03"}}),"26b03")
        with self.assertRaises(ValueError):read({"source":{"build_ver":"../escape"}})

    def test_upload_failure_does_not_publish_checksum(self):
        with tempfile.TemporaryDirectory() as tmp:
            images,log,env=self.upload_fixture(Path(tmp));env["MOCK_RSYNC_EXIT"]="23"
            result=subprocess.run(["bash",str(UPLOAD),str(images)],env=env,capture_output=True)
            self.assertEqual(result.returncode,23);self.assertEqual(len(log.read_text().splitlines()),1)

    def test_missing_credentials_and_empty_iso_fail(self):
        with tempfile.TemporaryDirectory() as tmp:
            images,log,env=self.upload_fixture(Path(tmp));env.pop("SF_SSH_PRIVATE_KEY")
            result=subprocess.run(["bash",str(UPLOAD),str(images)],env=env,capture_output=True);self.assertNotEqual(result.returncode,0);self.assertFalse(log.exists())
            env["SF_SSH_PRIVATE_KEY"]="fixture";(images/"Lingmo test.iso").write_bytes(b"")
            result=subprocess.run(["bash",str(UPLOAD),str(images)],env=env,capture_output=True);self.assertNotEqual(result.returncode,0);self.assertFalse(log.exists())

    def test_workflow_only_publishes_built_main_artifacts(self):
        workflow=yaml.safe_load((SOURCE/".github/workflows/build.yml").read_text());job=workflow["jobs"]["publish-sourceforge"]
        self.assertEqual(job["needs"],"build");self.assertIn("refs/heads/main",job["if"]);self.assertIn("iso-artifact-id",job["if"])

if __name__=="__main__":unittest.main()
