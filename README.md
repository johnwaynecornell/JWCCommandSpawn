JWCCommandSpawn
===============

**JWCCommandSpawn** is a hybrid native/.NET command spawning library for the NewAge-family workspace. It provides a C++17 shared library for launching and interacting with external commands and exposes a managed .NET wrapper for higher-level use from modern .NET applications.

The project is intended to work alongside [JWCEssentials](https://github.com/johnwaynecornell/JWCEssentials), which supplies shared native and managed utility types used throughout the NewAge environment.

Project Overview
----------------

*   **Native C++17 library:** builds `JWCCommandSpawn` as a shared library.
*   **.NET wrapper:** exposes the native command-spawning functionality to .NET through P/Invoke.
*   **Cross-platform intent:** includes platform-specific native implementations for Linux and Windows.
*   **Shell-aware execution:** supports shell selection such as default shell, Bash, and Python-oriented invocation.
*   **Pipe support:** supports stdin, stdout, and stderr style process communication.

Repository Layout
-----------------

    JWCCommandSpawn/
    ├── CMakeLists.txt
    ├── docs/
    │   └── NewAgeEnvironment.md
    ├── include/
    │   └── JWCCommandSpawn/
    ├── Project/
    │   └── JWCCommandSpawn.net/
    ├── src/
    │   ├── Platform_Linux/
    │   ├── Platform_Windows/
    │   └── ...
    └── LICENSE


Relationship to JWCEssentials
-----------------------------

JWCCommandSpawn depends on **JWCEssentials** for shared native and managed support code. The native build includes the JWCEssentials CMake integration from the NewAge workspace, and the .NET projects reference the staged `JWCEssentials.net.dll`.

JWCEssentials is expected to be available from:

    https://github.com/johnwaynecornell/JWCEssentials


This repository follows the same shared environment model documented in:

    docs/NewAgeEnvironment.md


In short, the projects are designed to live inside a shared `NewAge` workspace where native libraries, managed libraries, staging scripts, and environment variables agree on common paths.

NewAge Environment Requirement
------------------------------

The build expects a `NewAge` environment variable to point at the root of the shared NewAge workspace.

### Linux / Bash

    export NewAge="$HOME/NewAge"


### Windows PowerShell

    [Environment]::SetEnvironmentVariable("NewAge", "C:\src\NewAge", "User")


### Temporary PowerShell Session

    $env:NewAge = "C:\src\NewAge"


The native CMake build expects JWCEssentials integration to be available at a path similar to:

    $NewAge/include/JWCEssentials/JWCEssentials.cmake


The .NET projects expect staged managed dependencies under a path similar to:

    $(NewAge)\DotNet\Libs\lib\$(Configuration)\$(TargetFramework)


Native Build
------------

The native portion is built with CMake and C++17. It produces a shared library named `JWCCommandSpawn`.

### Native Requirements

*   CMake 3.28 or newer
*   A C++17-capable compiler
*   JWCEssentials built and discoverable in the NewAge workspace
*   The `NewAge` environment variable
*   Platform build tools, such as GCC/Clang on Linux or Visual Studio Build Tools on Windows

### Example Linux Build

    export NewAge="$HOME/NewAge"
    
    cmake -S . -B cmake-build-debug -DCMAKE_BUILD_TYPE=Debug
    cmake --build cmake-build-debug


### Example Windows Build

    $env:NewAge = "C:\src\NewAge"
    
    cmake -S . -B cmake-build-debug
    cmake --build cmake-build-debug --config Debug


The CMake project links against `JWCEssentials` and builds test executables in addition to the shared library.

.NET Build
----------

The managed side lives under:

    Project/JWCCommandSpawn.net/


It contains:

*   `JWCCommandSpawn.net` — the .NET wrapper library.
*   `TestCommandSpawn` — a small executable project for exercising the wrapper.

The .NET projects currently target:

*   `net8.0`
*   `net10.0`

### .NET Requirements

*   .NET SDK with the required target frameworks installed
*   `JWCEssentials.net.dll` staged into the NewAge managed library path
*   `bash` available for the post-build staging script
*   A valid `NewAge` environment variable

### Example .NET Build

    cd Project/JWCCommandSpawn.net
    
    dotnet build


During the managed library build, the project invokes a staging command similar to:

    bash NewAge_stage.sh "$(MSBuildProjectName)" "$(MyReferencePath)" "$(OutputPath)/"


This means the build environment must be able to locate the staging script and write to the shared NewAge staged library directory.

Runtime Notes
-------------

The managed wrapper uses P/Invoke to load the native library named `JWCCommandSpawn`. At runtime, the native shared library must be discoverable by the operating system or the .NET host.

Depending on platform and configuration, this may require one of the following:

*   Copying the native library beside the .NET executable
*   Adding the native build output directory to the platform library search path
*   Using the NewAge staging layout consistently across native and managed outputs

### Typical Native Library Names

*   Linux: `libJWCCommandSpawn.so`
*   Windows: `JWCCommandSpawn.dll`

Capabilities
------------

JWCCommandSpawn is designed around process execution and pipe-based interaction. The native library provides the low-level process handling, while the .NET wrapper exposes a more convenient managed interface.

### Core Features

*   Launch external commands
*   Select or configure a shell
*   Read process stdout
*   Read process stderr where supported by the selected pipe configuration
*   Write to process stdin
*   Join/wait for spawned commands
*   Use persistent command pipes for shell-like interaction

### Pipe Flags

The command API uses pipe flags for selecting which streams should be connected:

*   `E_PIPE_NONE`
*   `E_PIPE_STDOUT`
*   `E_PIPE_STDERR`
*   `E_PIPE_STDIN`

Example Managed Usage
---------------------

    using JWCCommandSpawn.net;
    
    CommandSpawn spawn = new CommandSpawn();
    
    string output = spawn.Exec("echo hello from JWCCommandSpawn");
    
    Console.WriteLine(output);


Example Bash-Oriented Pipe Usage
--------------------------------

    using JWCCommandSpawn.net;
    
    CommandSpawn spawn = new CommandSpawn();
    
    var bash = spawn.GetShell_Bash();
    spawn.SetShell(bash);
    
    spawn.Command("bash", CommandSpawn.E_PIPE.E_PIPE_STDIN | CommandSpawn.E_PIPE.E_PIPE_STDOUT);
    
    CommandPipe pipe = spawn.CreateCommandPipe();
    
    string result = pipe.Exec("echo persistent shell command");
    
    Console.WriteLine(result);
    
    spawn.Join();


Fresh System Checklist
----------------------

Before building on a new machine, verify the shared environment:

    echo "$NewAge"
    cmake --version
    dotnet --list-sdks
    dotnet --list-runtimes
    bash --version


On Windows PowerShell:

    echo $env:NewAge
    cmake --version
    dotnet --list-sdks
    dotnet --list-runtimes
    bash --version


Recommended Build Order
-----------------------

1.  Set up the shared `NewAge` workspace.
2.  Build and stage `JWCEssentials`.
3.  Build the native `JWCCommandSpawn` shared library with CMake.
4.  Build the managed `JWCCommandSpawn.net` project.
5.  Run the native or managed test projects as needed.

Troubleshooting
---------------

### JWCEssentials cannot be found

Confirm that JWCEssentials has been cloned, built, and staged according to the NewAge environment layout. Also verify that the `NewAge` variable points to the expected workspace root.

### .NET cannot resolve JWCEssentials.net.dll

Check the staged managed library path:

    $(NewAge)\DotNet\Libs\lib\$(Configuration)\$(TargetFramework)


Ensure that `JWCEssentials.net.dll` exists for the configuration and target framework being built.

### P/Invoke cannot load JWCCommandSpawn

Make sure the native shared library has been built and is available in a runtime-searchable location. For a quick test, copy the native library beside the .NET executable output.

### Post-build staging fails

Confirm that `bash` is installed and that `NewAge_stage.sh` is resolvable from the build context.

License
-------

See `LICENSE` for licensing information.