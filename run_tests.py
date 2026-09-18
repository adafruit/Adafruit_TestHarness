#!/usr/bin/env python3
"""Compile and run host C++ regressions with memory/undefined-behavior checks."""
import argparse
import os
from pathlib import Path
import subprocess
import tempfile


def run_tests(tests, sources=(), includes=()):
    """Build each test as a separate executable with the shared Arduino shim.

    Tests must define main(). Select optional peripheral fixtures through
    includes; the generic bus fakes use callbacks for device-specific behavior.
    """
    root = Path(__file__).resolve().parent
    tests = sorted(Path(test).resolve() for test in tests)
    if not tests:
        raise ValueError("No test sources found")
    sources = [root / "src/Arduino.cpp", *map(Path, sources)]
    for path in [*tests, *sources]:
        if not path.is_file():
            raise FileNotFoundError(path)
    flags = ["-std=c++11", "-Wall", "-Wextra", "-Werror", "-g",
             "-fsanitize=address,undefined", "-fno-sanitize-recover=all",
             "-fno-omit-frame-pointer", "-fno-pie", "-no-pie"]
    flags.extend("-I" + str(Path(path).resolve()) for path in includes)
    flags.append("-I" + str(root / "include"))
    failures = []
    with tempfile.TemporaryDirectory(prefix="adafruit-tests-") as build:
        for index, test in enumerate(tests):
            binary = str(Path(build) / str(index))
            print("TEST", test, flush=True)
            try:
                subprocess.run([os.environ.get("CXX", "g++"), *flags,
                                *map(str, sources), str(test), "-o", binary],
                               check=True, timeout=120)
                subprocess.run([binary], check=True, timeout=30)
            except (subprocess.CalledProcessError,
                    subprocess.TimeoutExpired) as error:
                print("FAIL", test, error, flush=True)
                failures.append(test)
            else:
                print("PASS", test, flush=True)
    print(f"{len(tests) - len(failures)}/{len(tests)} test sources passed.")
    if failures:
        raise SystemExit(1)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--test-dir", type=Path,
                        default=Path(__file__).resolve().parent / "tests")
    parser.add_argument("--source", type=Path, action="append", default=[])
    parser.add_argument("--include", type=Path, action="append", default=[])
    args = parser.parse_args()
    run_tests(args.test_dir.rglob("*.cpp"), args.source, args.include)


if __name__ == "__main__":
    main()
