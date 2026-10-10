"""Black-box command-line tests for the student score manager."""

import os
import shutil
import subprocess
import tempfile
import unittest
from pathlib import Path


PROJECT_ROOT = Path(__file__).resolve().parents[1]
SOURCE = PROJECT_ROOT / "student-management" / "main.c"
TEST_TEMP_ROOT = PROJECT_ROOT / "tests" / ".tmp"


class ScoreManagerCliTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        TEST_TEMP_ROOT.mkdir(parents=True, exist_ok=True)
        cls.build_dir = tempfile.TemporaryDirectory(
            prefix="score-manager-build-", dir=TEST_TEMP_ROOT
        )
        exe_name = "score_manager.exe" if os.name == "nt" else "score_manager"
        cls.executable = Path(cls.build_dir.name) / exe_name
        cls.compiler_kind = None
        errors = []

        for compiler_name in ("gcc", "clang", "cl"):
            compiler = shutil.which(compiler_name)
            if compiler is None:
                continue

            if compiler_name == "cl":
                command = [
                    compiler,
                    "/nologo",
                    "/std:c11",
                    "/utf-8",
                    "/W4",
                    "/TC",
                    str(SOURCE),
                    "/Fo" + str(Path(cls.build_dir.name)) + os.sep,
                    "/Fe:" + str(cls.executable),
                ]
            else:
                command = [
                    compiler,
                    "-std=c11",
                    "-Wall",
                    "-Wextra",
                    "-Wpedantic",
                    "-O2",
                    str(SOURCE),
                    "-o",
                    str(cls.executable),
                ]

            try:
                result = subprocess.run(
                    command,
                    capture_output=True,
                    text=True,
                    encoding="utf-8",
                    errors="replace",
                    timeout=60,
                )
            except (OSError, subprocess.TimeoutExpired) as error:
                errors.append(f"{compiler_name}: {error}")
                continue

            if result.returncode == 0:
                cls.compiler_kind = compiler_name
                break
            errors.append(
                f"{compiler_name} exited {result.returncode}:\n"
                f"{result.stdout}\n{result.stderr}"
            )

        if cls.compiler_kind is None:
            cls.build_dir.cleanup()
            raise RuntimeError(
                "Could not compile with GCC, Clang, or MSVC. On Windows, "
                "open an x64 Native Tools Command Prompt for VS 2022 first.\n"
                + "\n".join(errors)
            )

    @classmethod
    def tearDownClass(cls):
        if hasattr(cls, "build_dir"):
            cls.build_dir.cleanup()
        try:
            TEST_TEMP_ROOT.rmdir()
        except OSError:
            pass

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

    def temp_working_directory(self):
        return tempfile.TemporaryDirectory(
            prefix="score-manager-case-", dir=TEST_TEMP_ROOT
        )

    def test_exit_from_menu(self):
        with self.temp_working_directory() as cwd:
            result = self.run_program("7\n", cwd)
        self.assertEqual(result.returncode, 0)
        self.assertIn("系统已退出", result.stdout)

    def test_invalid_and_oversized_menu_inputs_do_not_exit(self):
        with self.temp_working_directory() as cwd:
            result = self.run_program("abc\n8\n4294967295\n7\n", cwd)
        self.assertEqual(result.returncode, 0)
        self.assertGreaterEqual(result.stdout.count("输入无效"), 3)
        self.assertIn("系统已退出", result.stdout)

    def test_add_search_update_statistics_and_delete(self):
        inputs = "\n".join(
            [
                "1", "1001", "Alice", "CS1", "90", "80", "70",
                "3", "lic", "4", "1001", "2", "100", "5",
                "6", "1001", "2", "7", "",
            ]
        )
        with self.temp_working_directory() as cwd:
            result = self.run_program(inputs, cwd)
        self.assertEqual(result.returncode, 0)
        self.assertIn("学生信息已添加并保存", result.stdout)
        self.assertIn("共找到 1 位学生", result.stdout)
        self.assertIn("平均分：80.00", result.stdout)
        self.assertIn("高等数学班级平均分：100.00", result.stdout)
        self.assertIn("学生记录已删除并保存", result.stdout)
        self.assertIn("目前没有学生记录", result.stdout)

    def test_exact_field_limit_is_accepted_and_longer_input_is_rejected(self):
        name_at_limit = "A" * 49
        too_long_name = "B" * 50
        inputs = "\n".join(
            ["1", "3001", too_long_name, name_at_limit, "CS", "0", "100", "80.5", "7", ""]
        )
        with self.temp_working_directory() as cwd:
            result = self.run_program(inputs, cwd)
            csv_file = Path(cwd) / "students.csv"
            self.assertTrue(csv_file.exists(), repr(result.stdout) + repr(inputs))
            saved = csv_file.read_text(encoding="utf-8")
        self.assertEqual(result.returncode, 0)
        self.assertIn("输入过长", result.stdout)
        self.assertIn("学生信息已添加并保存", result.stdout)
        self.assertIn(f"3001,{name_at_limit},CS,0,100,80.5", saved)

    def test_invalid_scores_are_rejected_and_boundaries_are_accepted(self):
        inputs = "\n".join(
            ["1", "7001", "Boundary", "CS", "-1", "101", "nan", "0", "100", "12.5", "7", ""]
        )
        with self.temp_working_directory() as cwd:
            result = self.run_program(inputs, cwd)
            saved = (Path(cwd) / "students.csv").read_text(encoding="utf-8")
        self.assertEqual(result.returncode, 0)
        self.assertGreaterEqual(result.stdout.count("请输入 0 到 100 之间的有效数字"), 3)
        self.assertIn("7001,Boundary,CS,0,100,12.5", saved)

    def test_csv_data_loads_on_next_run(self):
        with self.temp_working_directory() as cwd:
            first = self.run_program("1\n2001\nBob\nCS2\n88\n77\n66\n7\n", cwd)
            csv_file = Path(cwd) / "students.csv"
            saved_csv = csv_file.read_text(encoding="utf-8")
            second = self.run_program("2\n7\n", cwd)
        self.assertEqual(first.returncode, 0)
        self.assertEqual(second.returncode, 0)
        self.assertIn("已从 students.csv 载入 1 位学生", second.stdout)
        self.assertIn("Bob", second.stdout)
        self.assertIn("2001,Bob,CS2,88,77,66", saved_csv)

    def test_malformed_trailing_data_and_duplicate_csv_ids_are_skipped(self):
        with self.temp_working_directory() as cwd:
            csv_file = Path(cwd) / "students.csv"
            csv_file.write_text(
                "5001,Valid,CS,50,60,70\n"
                "5002,Trailing,CS,50,60,70,extra\n"
                "5001,Duplicate,CS,1,2,3\n"
                "5003,OutOfRange,CS,101,20,30\n",
                encoding="utf-8",
            )
            result = self.run_program("2\n7\n", cwd)
        self.assertEqual(result.returncode, 0)
        self.assertIn("有 3 条无效记录", result.stdout)
        self.assertIn("Valid", result.stdout)
        self.assertNotIn("Trailing", result.stdout)
        self.assertNotIn("Duplicate", result.stdout)
        self.assertNotIn("OutOfRange", result.stdout)

    def test_empty_statistics_are_reported_without_division(self):
        with self.temp_working_directory() as cwd:
            result = self.run_program("5\n7\n", cwd)
        self.assertEqual(result.returncode, 0)
        self.assertIn("目前没有学生记录，无法计算平均成绩", result.stdout)

    def test_failed_save_keeps_memory_and_original_csv_unchanged(self):
        original = "6001,Before,CS1,50,60,70\n"
        inputs = "\n".join(
            [
                "1", "6002", "After", "CS2", "1", "2", "3",
                "2", "4", "6001", "1", "90", "2", "6", "6001", "2", "7", "",
            ]
        )
        with self.temp_working_directory() as cwd:
            data_file = Path(cwd) / "students.csv"
            data_file.write_text(original, encoding="utf-8")
            (Path(cwd) / "students.csv.tmp").mkdir()
            result = self.run_program(inputs, cwd)
            saved = data_file.read_text(encoding="utf-8")
        self.assertEqual(result.returncode, 0)
        self.assertEqual(result.stdout.count("无法创建临时数据文件"), 3)
        self.assertIn("Before", result.stdout)
        self.assertNotIn("After", result.stdout)
        self.assertEqual(saved, original)

    def test_duplicate_student_id_is_rejected(self):
        original = "8001,Original,CS,70,80,90\n"
        inputs = "\n".join(["1", "8001", "Duplicate", "CS", "2", "2", "2", "7", ""])
        with self.temp_working_directory() as cwd:
            data_file = Path(cwd) / "students.csv"
            data_file.write_text(original, encoding="utf-8")
            result = self.run_program(inputs, cwd)
            saved = data_file.read_text(encoding="utf-8")
        self.assertEqual(result.returncode, 0)
        self.assertIn("该学号已存在", result.stdout)
        self.assertEqual(saved, original)


if __name__ == "__main__":
    unittest.main()
