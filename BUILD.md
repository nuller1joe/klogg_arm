# How to Build Klogg

## Overview

These instructions will get you a copy of the project up and running on your local machine for development and testing purposes.
Local builds can be faster because code can be optimized for current CPU instead of generic x86-64. Support for SSE4/AVX code paths
will be enabled if available on build machine.

## Getting the Source

This project is [hosted on GitHub](https://github.com/variar/klogg). You can clone this project directly using this command:

```
git clone https://github.com/variar/klogg
```

## Dependencies

To build Klogg:

- cmake 3.12 or later to generate build files
- C++ compiler with decent C++17 support (at least gcc 7.5, clang 7, msvc 19.14)
- Qt libraries 5.9 or later (CI builds use Qt 5.9.5/5.12.5/5.15.2):
  - QtCore
  - QtGui
  - QtWidgets
  - QtConcurrent
  - QtNetwork
  - QtXml
  - QtTools

To build Hyperscan regular expressions backend (default):

- CPU with support for [SSSE3](https://en.wikipedia.org/wiki/SSSE3) instructions (for Hyperscan backend)
- Boost (1.58 or later, header-only part)
- Ragel (6.8 or later; precompiled binary is provided for Windows; has to be installed from package managers on Linux or Homebrew on Mac)

To build installer for Windows:

- nsis to build installer for Windows
- Precompiled OpenSSl library to enable https support on Windows

Building tests:

- QtTest

All other dependencies are provided by [CPM](https://github.com/cpm-cmake/CPM.cmake) during cmake configuration stage (see 3rdparty directory).

CPM will try to find Hyperscan, TBB, uchardet and xxhash installed on build host.
If a library can't be found, the one provided by CPM will be used.

## Building

### Building on Apple Silicon

Use an ARM64 terminal and ARM64 Homebrew in `/opt/homebrew`, with Xcode's
command-line tools installed. The preset requires CMake 3.21 or newer.

```sh
brew install cmake ninja qtbase qt5compat qttools qtsvg boost ragel pkgconf
cmake --preset macos-arm64
cmake --build --preset macos-arm64
ctest --preset macos-arm64
open build-arm64/output/klogg.app
```

Configuration downloads the pinned source dependencies and therefore needs
internet access on the first run. The preset builds a native ARM64 Release app
with Vectorscan search support and the test executables. Generic ARM CPU flags
avoid optimizing the app exclusively for the Mac on which it was compiled.
The preset also enables the compatibility policy needed by older dependencies
when using CMake 4.

The app initially links to Homebrew's Qt frameworks. To bundle those frameworks
for a local standalone copy, run:

```sh
python3 packaging/osx/deploy-local.py build-arm64/output/klogg.app
file build-arm64/output/klogg.app/Contents/MacOS/klogg
```

The deployment helper includes both the native Cocoa platform plugin and the
offscreen plugin used by the smoke test, applies a local ad-hoc signature, and
checks startup with each plugin. Keep the entire `.app` bundle together when
moving it. Rerun this helper after rebuilding the app.

Signing and notarization for distribution are separate from this local build.
The preset uses Homebrew's current SDK/dependency deployment requirements; it
does not guarantee compatibility with older macOS releases.

### Configuration options

By default Klogg is built without support for reporting crash dumps. This can be enabled via cmake option `-DKLOGG_USE_SENTRY=ON`.

Klogg uses Hyperscan regular expressions library which requires CPU with SSSE3 support, ragel and boost headers.
Klogg can be built with only Qt reqular expressions backend by passing `-DKLOGG_USE_HYPERSCAN=OFF` to cmake.

Klogg can use custom memory allocator. By default it uses TBB memory allocator for Windows, mimalloc on Linux and default system allocator on MacOS.
Memory allocator override can be turned off by passing `-DKLOGG_OVERRIDE_MALLOC`. If you want to use TBB allocator on Linux then pass
`-DKLOGG_USE_MIMALLOC=OFF`.

### Building on Linux

Here is how to build klogg on Ubuntu 18.04.

Install dependencies:

```
sudo apt-get install build-essential cmake qtbase5-dev libboost-all-dev ragel
```

Configure and build klogg:

```
cd <path_to_klogg_repository_clone>
mkdir build_root
cd build_root
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo ..
cmake --build .
```

**_If cmake gives error about missing "Qt5LinguistTools" configuration files, try running:_**

```bash
sudo apt-get install qttools5-dev
```

Binaries are placed into `build_root/output`.

See `.github/workflows/ci-build.yml` for more information on build process.

### Building on Windows

Install Microsoft Visual Studio 2017 or 2019 with C++ support.
Community edition can be downloaded from [Microsoft](https://visualstudio.microsoft.com/vs/).

Intall latest Qt version using [online installer](https://www.qt.io/download-qt-installer).
Make sure to select version matching Visual Studio installation. 64-bit libraries are recommended.

Install CMake from [Kitware](https://cmake.org/download/).
Use version 3.14 or later for Visual Studio 2019 support.

Download the Boost source code from http://www.boost.org/users/download/.
Extract to some folder. Directory structure should be something like `C:\Boost\boost_1_63_0`.
Then add `BOOST_ROOT` environment variable pointing to main directory of Boost sources so CMake is able to fine it.

Prepare build environment for CMake. Open command prompt window and depending on version of Visual Studio run either

```
call "%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Community\Common7\Tools\vsdevcmd" -arch=x64
```

or

```
call "%ProgramFiles(x86)%\Microsoft Visual Studio\2017\Community\Common7\Tools\vsdevcmd" -arch=x64
```

Next setup Qt paths:

```
<path_to_qt_installation>\bin\qtenv2.bat
```

Then add CMake to PATH:

```
set PATH=<path_to_cmake_bin>:$PATH
```

Configure klogg solution (use CMake generator matching Visual Studio version):

```
cd <path_to_project_root>
md build_root
cd build_root
cmake -G "Visual Studio 16 2019 Win64" -DCMAKE_BUILD_TYPE=RelWithDebInfo ..
```

CMake should generate `klogg.sln` file in `<path_to_project_root>\build_root` directory. Open solution and build it.

Binaries are placed into `build_root/output`.

For https network urls support download precompiled openssl library https://mirror.firedaemon.com/OpenSSL/openssl-1.1.1l-dev.zip.
Put libcrypto-1_1 and libssl-1_1 for desired architecture near klogg binaries.

### Building on Mac OS

For Apple Silicon and Qt 6, use [Building on Apple Silicon](#building-on-apple-silicon).
The instructions below describe the older Intel / Qt 5 setup. Its original
minimum macOS version was High Sierra (10.13); current dependencies may require
a newer macOS version.

Install [Homebrew](https://brew.sh/) using terminal:

```
/usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
```

Homebrew installer should also install xcode command line tools.

Download and install build dependencies:

```
brew install cmake ninja qt boost ragel
```

Usually path to qt installation looks like `/usr/local/Cellar/qt/5.14.0/lib/cmake/Qt5`

Configure and build klogg:

```
cd <path_to_klogg_repository_clone>
mkdir build_root
cd build_root
cmake -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo -DQt5_DIR=<path_to_qt_install> ..
cmake --build .
```

Binaries are placed into `build_root/output`.

By default, klogg will rely on cmake to figure out target MacOS version. Usually it uses build host version.
To override default cmake value pass an option `-DKLOGG_OSX_DEPLOYMENT_TARGET=<target>` to cmake during configuration step,
`<target>` is one of `10.14`, `10.15`, `11`, `12`. Klogg's traget must be greater or equal to target used by Qt libraries.

## Running tests

Tests are built by default. To turn them off pass `-DBUILD_TESTS:BOOL=OFF` to cmake.
Tests use catch2 (bundled with klogg sources) and require Qt5Test module. Tests can be run using ctest tool provider by CMake:

```
cd <path_to_klogg_repository_clone>
cd build_root
ctest --build-config RelWithDebInfo --verbose
```
