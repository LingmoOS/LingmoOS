import importlib.util
from pathlib import Path
import tempfile
import unittest
spec=importlib.util.spec_from_file_location("restore_aliases",Path(__file__).resolve().parents[1]/"tools/restore-aliases.py")
module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
class Aliases(unittest.TestCase):
    def test_flattened_alias_chain(self):
        with tempfile.TemporaryDirectory() as tmp:
            root=Path(tmp);(root/"art.svg").write_text('<svg xmlns="http://www.w3.org/2000/svg"/>')
            (root/"first.svg").write_text("second.svg\n");(root/"second.svg").write_text("art.svg\n")
            self.assertEqual(module.restore(root),2)
            self.assertEqual((root/"first.svg").read_text(),(root/"art.svg").read_text())
            self.assertEqual(module.restore(root),0)
    def test_rejects_escape(self):
        with tempfile.TemporaryDirectory() as tmp:
            root=Path(tmp);(root/"bad.svg").write_text("../outside.svg")
            with self.assertRaises(ValueError):module.restore(root)
if __name__=="__main__":unittest.main()
