# Schunk SVH Driver Installation Guide

## Installing the Schunk SVH Driver on Windows Subsystem for Linux (WSL)

### Step 1: Install WSL

Open PowerShell as an Administrator and run the following command:
```powershell
wsl --install -d ubuntu
```
During the installation, you'll be prompted to create a username and password.

### Step 2: Continue with the steps below

## Installing the Schunk SVH Driver on Ubuntu

### Step 1: Install Necessary Dependencies

```sh
sudo apt update
sudo snap install cmake --classic
sudo apt install git build-essential gdb libboost-test-dev openjdk-21-jdk -y
```

### Step 3: Clone the Schunk SVH Driver Project
Clone the repository and navigate into the project directory:
```sh
git clone https://github.com/Sinker01/schunk_svh_library_windows
cd schunk_svh_library_windows
```

### Step 4: Build and Install the Driver
Create a build directory, configure the project, build, and install:
```sh
mkdir build
cd build 
cmake ..
cmake --build .
sudo cmake --install .
```

### Step 5: Attach USB Drive to WSL (Optional)
If you want to run the program and need access to a USB drive, attach it to WSL.
```powershell
```

## Installing the Schunk SVH Driver on Windows

### Step 1: Install MSYS2 and MinGW
Follow the instructions on the [MSYS2](https://www.msys2.org/) website to install MSYS2 and MinGW.

### Step 2: Install Necessary Tools
Open PowerShell as an Administrator and run the following commands:
```powershell
winget install Git.Git Kitware.CMake Ninja-build.Ninja Oracle.JDK.22 7Zip.7Zip
```

### Step 3: Install Boost
Create a directory for Boost, download, and extract it:
```powershell
mkdir C:\local
cd C:\local
curl -O https://archives.boost.io/release/1.82.0/source/boost_1_85_0.7z
cmd.exe /c "C:\Program Files\7-Zip\7z.exe" x boost_1_85_0.7z
cd boost_1_85_0
.\bootstrap.bat mingw
.\b2 toolset=gcc address-model=64 --with-test
```

### Step 4: Clone the Schunk SVH Driver Project
Navigate to your chosen directory and clone the project:
```powershell
git clone https://github.com/Sinker01/schunk_svh_library_windows
```

Step 5: Build and Install the Driver
Open a new terminal as Administrator, navigate to the project directory, and run the following commands:
```powershell
mkdir build
cd build 
cmake -G "MinGW Makefiles" ..
cmake --build .
cmake --install .
```