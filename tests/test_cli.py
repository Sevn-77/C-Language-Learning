"""Black-box command-line tests for the student score manager."""

import shutil
import subprocess
import tempfile
import unittest
from pathlib import Path


PROJECT_ROOT = Path(__file__).resolve().parents[1]
SOURCE = PROJECT_ROOT / "student-management" / "main.c"


class ScoreManagerCliTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.compiler = shutil.which("gcc")
        if cls.compiler is None:
            raise RuntimeError("GCC was not found on PATH")
        cls.build_dir = tempfile.TemporaryDirectory(prefix="score-manager-build-")
        exe_name = "score_manager.exe" if shutil.which("cmd") else "score_manager"
        cls.executable = Path(cls.build_dir.name) / exe_name
        result = subprocess.run(
            [cls.compiler, "-std=c11", "-Wall", "-Wextra", "-Wpedantic", "-O2",
             str(SOURCE), "-o", str(cls.executable)],
            capture_output=True,
            text=True,
            encoding="utf-8",
            errors="replace",
        )
        if result.returncode != 0:
            raise RuntimeError(f"GCC compilation failed:\n{result.stderr}")

    @classmethod
    def tearDownClass(cls):
        if hasattr(cls, "build_dir"):
            cls.build_dir.cleanup()

    def run_program(self, inputs, cwd):
        return subprocess.run(
            [str(self.executable)],
            input=inputs,
            cwd=cwd,
            capture_output=True,
            text=True,
            encoding="utf-8",
            errors="replace",
            timeout=5,
        )

    def test_exit_from_menu(self):
        with tempfile.TemporaryDirectory() as cwd:
            result = self.run_program("7\n", cwd)
        self.assertEqual(result.returncode, 0)
        self.assertIn("系统已退出", result.stdout)

    def test_invalid_menu_inputs_do_not_exit(self):
        with tempfile.TemporaryDirectory() as cwd:
            result = self.run_program("abc\n8\n7\n", cwd)
        self.assertEqual(result.returncode, 0)
        self.assertGreaterEqual(result.stdout.count("输入无效"), 2)
        self.assertIn("系统已退出", result.stdout)

    def test_add_search_update_statistics_and_delete(self):
        inputs = "\n".join([
            "1", "1001", "Alice", "CS1", "90", "80", "70",
            "3", "lic", "4", "1001", "2", "100", "5", "6", "1001", "2", "7", "",
        ])
        with tempfile.TemporaryDirectory() as cwd:
            result = self.run_program(inputs, cwd)
        self.assertEqual(result.returncode, 0)
        self.assertIn("学生信息已添加并保存", result.stdout)
        self.assertIn("共找到 1 位学生", result.stdout)
        self.assertIn("平均分：80.00", result.stdout)
        self.assertIn("高等数学班级平均分：100.00", result.stdout)
        self.assertIn("学生记录已删除并保存", result.stdout)
        self.assertIn("目前没有学生记录", result.stdout)

    def test_csv_data_loads_on_next_run(self):
        with tempfile.TemporaryDirectory() as cwd:
            first = self.run_program("1\n2001\nBob\nCS2\n88\n77\n66\n7\n", cwd)
            csv_file = Path(cwd) / "students.csv"
            self.assertTrue(csv_file.exists())
            saved_csv = csv_file.read_text(encoding="utf-8")
            second = self.run_program("2\n7\n", cwd)
        self.assertEqual(first.returncode, 0)
        self.assertEqual(second.returncode, 0)
        self.assertIn("已从 students.csv 载入 1 位学生", second.stdout)
        self.assertIn("Bob", second.stdout)
        self.assertIn("2001,Bob,CS2,88,77,66", saved_csv)


if __name__ == "__main__":
    unittest.main()

