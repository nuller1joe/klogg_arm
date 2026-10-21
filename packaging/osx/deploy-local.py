#!/usr/bin/env python3
"""Bundle Homebrew Qt, including the platform plugin used by the smoke test."""

import argparse
import os
from pathlib import Path
import shutil
import subprocess


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("app", type=Path)
    args = parser.parse_args()
    app = args.app.resolve()
    executable = app / "Contents/MacOS/klogg"
    if not executable.is_file():
        parser.error(f"Not a built Klogg app: {app}")

    qt_libs = subprocess.check_output(
        ["qtpaths6", "--query", "QT_INSTALL_LIBS"], text=True
    ).strip()
    qt_plugins = Path(subprocess.check_output(
        ["qtpaths6", "--query", "QT_INSTALL_PLUGINS"], text=True
    ).strip())
    platforms = app / "Contents/PlugIns/platforms"
    platforms.mkdir(parents=True, exist_ok=True)
    offscreen = platforms / "libqoffscreen.dylib"
    shutil.copy2(qt_plugins / "platforms/libqoffscreen.dylib", offscreen)

    # Homebrew splits Qt modules across prefixes. Supply the shared library
    # directory explicitly so plugins such as QtSvg are resolved as well.
    deployment = subprocess.run(
        ["macdeployqt", str(app), f"-libpath={qt_libs}",
         f"-executable={offscreen}", "-always-overwrite", "-no-strip", "-no-codesign"],
        stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True, check=True
    )
    print(deployment.stdout, end="", flush=True)
    # macdeployqt can log dependency errors while returning success.
    if "ERROR:" in deployment.stdout:
        raise RuntimeError("Qt deployment reported errors; see output above")

    subprocess.run(["codesign", "--force", "--deep", "--sign", "-", str(app)], check=True)
    subprocess.run(["codesign", "--verify", "--deep", "--strict", str(app)], check=True)

    # Verify the bundled plugins without inheriting development Qt paths.
    env = {key: value for key, value in os.environ.items()
           if not key.startswith(("QT_", "DYLD_"))}
    for platform in ("cocoa", "offscreen"):
        result = subprocess.run(
            [str(executable), "-platform", platform, "-v"], env=env,
            stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True,
            timeout=15, check=True
        )
        if "klogg " not in result.stdout:
            raise RuntimeError(f"Startup check failed for {platform}: {result.stdout}")
        print(f"{platform} startup passed: {result.stdout.strip()}", flush=True)


if __name__ == "__main__":
    main()
