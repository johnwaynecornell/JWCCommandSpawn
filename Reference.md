# Reference.md

## Purpose
This document serves as an orientation guide for AI assistants and maintainers of **JWCCommandSpawn**. It maps the repository's structure, exported entry points, shell and pipe abstractions, and safe extension points. This is a practical map, not exhaustive API documentation.

## Repository Overview
**JWCCommandSpawn** is a cross-platform process-spawning library that abstracts `fork`/`exec` (Linux) and `CreateProcess` (Windows) behind a unified C++ API, with full stdin/stdout/stderr pipe management and a .NET P/Invoke wrapper.

- **Primary Languages**: C++17, C#/.NET 10.0
- **Target Platforms**: Windows and Linux
- **Dependency**: JWCEssentials only (`utf8_string_struct`, `_EXPORT_` macros, platform utilities)
- **Major Components**:
    - **Native library** (`libJWCCommandSpawn.so` / `JWCCommandSpawn.dll`): core process management and pipe I/O
    - **Platform implementations**: `src/Platform_Linux/` (fork/exec/pipe) and `src/Platform_Windows/` (CreateProcess)
    - **Managed .NET wrapper**: `Project/JWCCommandSpawn.net/JWCCommandSpawn.net/`
    - **Tests**: `src/test*.cpp` (native), `Project/JWCCommandSpawn.net/TestCommandSpawn/` and `TestNested_feffect/` (managed)
- **Primary Solution/Project Files**:
    - `CMakeLists.txt` (native build)
    - `Project/JWCCommandSpawn.net/JWCCommandSpawn.net.sln` (.NET solution)

## Orientation for AI Assistants
- **Prefer `utf8_string_struct` for all strings**: all string parameters and returns use `utf8_string_struct` from JWCEssentials — do not introduce `std::string` at the export boundary.
- **`_EXPORT_` must stay single-line**: grep-friendliness is a hard requirement.
- **Escapement matters**: never pass user-supplied strings to `CommandSpawn_Command` without considering the active `EscapementStyle`. Use `CommandSpawn_ToString` to preview the final command string.
- **Pipe flags are bitwise**: `E_PIPE` values compose — `E_PIPE_STDIN | E_PIPE_STDOUT` is valid.
- **Repo-relative paths only**: use repo-relative paths in notes, documentation, and when referencing symbols.
- **Platform specificity**: `src/Platform_Linux/` and `src/Platform_Windows/` are platform-specific. The shared core in `src/CommandSpawn.cpp` dispatches to them.

## Build / Runtime Assumptions
- **JWCEssentials required**: `$NewAge/include/JWCEssentials/JWCEssentials.cmake` must be present. Run `configure.sh` in JWCEssentials before JWCCommandSpawn.
- **configure.sh**: registers `include/JWCCommandSpawn` into `$NewAge/include/JWCCommandSpawn` via symlink. Requires `$NewAge` to be set.
- **Native library**: built via CMake, staged to `$NewAge/lib/<Config>/<Lane>/`. Managed wrapper must find it at runtime (`LD_LIBRARY_PATH` on Linux, PATH on Windows).
- **`BUILD_SPAWNEDPROCESS`**: defined at compile time to flip `_EXPORT_` from import to export. Set automatically by CMake via `add_definitions(-DBUILD_SPAWNEDPROCESS)`.
- **Process lifetime**: `CommandSpawn_Create` allocates a `CommandSpawn` instance. `CommandSpawn_Join` blocks until the process exits and returns the exit code. `CommandSpawn_Destroy` must always be called to free resources.

## Key Types

### `CommandSpawn::Shell`
Identifies which shell interpreter is used when running commands.

| Value | Meaning |
|---|---|
| Default | Platform default (bash on Linux, cmd.exe on Windows) |
| Bash | Explicit bash shell |
| Python | Python interpreter |

### `CommandSpawn::EscapementStyle`
Controls how command strings are escaped before being passed to the shell.

| Value | Meaning |
|---|---|
| Auto | Automatically inferred from the active shell |
| None | No escaping applied |
| PosixShell | POSIX shell escaping (`'...'` quoting) |
| WindowsCommandLine | Win32 command-line escaping |
| CmdExe | cmd.exe-style escaping |
| PowerShell | PowerShell string escaping |

### `CommandSpawn::E_PIPE`
Bitfield selecting which I/O streams to open as pipes.

| Flag | Meaning |
|---|---|
| `E_PIPE_STDIN` | Pipe stdin (allows `CommandSpawn_WriteString` etc.) |
| `E_PIPE_STDOUT` | Pipe stdout (allows `CommandSpawn_ReadLine` etc.) |
| `E_PIPE_STDERR` | Pipe stderr |

Flags compose: `E_PIPE_STDOUT | E_PIPE_STDERR` captures both output streams.

## Native / Exported Entry Points
All symbols in `include/JWCCommandSpawn/CommandSpawn.h`.

### Lifecycle

| Symbol | Purpose | Notes |
|---|---|---|
| `_EXPORT_ CommandSpawn_Create()` | Allocates a new CommandSpawn instance. | Returns opaque pointer. Must be destroyed. |
| `_EXPORT_ CommandSpawn_Destroy(This)` | Frees a CommandSpawn instance. | Always call after use. |

### Shell Configuration

| Symbol | Purpose | Notes |
|---|---|---|
| `_EXPORT_ CommandSpawn_GetShell_Default(This)` | Returns the platform default shell. | |
| `_EXPORT_ CommandSpawn_GetShell_Bash(This)` | Returns the Bash shell descriptor. | |
| `_EXPORT_ CommandSpawn_GetShell_Python(This)` | Returns the Python interpreter descriptor. | |
| `_EXPORT_ CommandSpawn_HasShell(This, shell)` | Returns true if the given shell is available. | Checks PATH for the interpreter. |
| `_EXPORT_ CommandSpawn_SetShell(This, shell)` | Sets the active shell by enum. | |
| `_EXPORT_ CommandSpawn_SetShellExplicit(This, name, shell, shell_switch)` | Sets the active shell to an arbitrary interpreter. | `shell_switch` is the flag passed before the command (e.g., `-c`). |

### Escapement

| Symbol | Purpose | Notes |
|---|---|---|
| `_EXPORT_ CommandSpawn_SetEscapementStyle(This, style)` | Sets the active escapement style. | |
| `_EXPORT_ CommandSpawn_GetEscapementStyle(This)` | Returns the active escapement style. | |
| `_EXPORT_ CommandSpawn_escapeStringForCommandLine(value, style)` | Escapes a string for the given style. | Stateless utility; does not require an instance. |
| `_EXPORT_ CommandSpawn_ToString(This, command)` | Returns the full command string as it would be executed. | Use before `Command` to preview/log. |

### Execution and Pipes

| Symbol | Purpose | Notes |
|---|---|---|
| `_EXPORT_ CommandSpawn_Command(This, command, for_stdin, pipes)` | Spawns a process with the given command. | `for_stdin` is written to stdin if `E_PIPE_STDIN` is not set. Non-blocking. |
| `_EXPORT_ CommandSpawn_Join(This)` | Blocks until the spawned process exits. | Returns exit code. |
| `_EXPORT_ CommandSpawn_ClosePipe(This, pipes)` | Closes specified pipe(s). | Call to signal EOF to the child process stdin. |

### Pipe I/O

| Symbol | Purpose | Notes |
|---|---|---|
| `_EXPORT_ CommandSpawn_HasData(This, targ)` | Returns true if the pipe has data to read. | Non-blocking check. |
| `_EXPORT_ CommandSpawn_ReadByte(This, targ)` | Reads one byte from the specified pipe. | Returns -1 on EOF/error. |
| `_EXPORT_ CommandSpawn_ReadLine(This, targ)` | Reads a line from the specified pipe. | Blocks until newline or EOF. |
| `_EXPORT_ CommandSpawn_ReadToEnd(This, targ)` | Reads all remaining data from the pipe. | Blocks until process closes the pipe. |
| `_EXPORT_ CommandSpawn_WriteByte(This, byte)` | Writes one byte to stdin. | Requires `E_PIPE_STDIN`. |
| `_EXPORT_ CommandSpawn_WriteString(This, string)` | Writes a string to stdin. | |
| `_EXPORT_ CommandSpawn_WriteLine(This, line)` | Writes a string followed by a newline to stdin. | |

## Managed Class Map

### `CommandSpawn` (managed wrapper)
File: `Project/JWCCommandSpawn.net/JWCCommandSpawn.net/CommandSpawn.cs`

Purpose: .NET P/Invoke wrapper exposing the full native CommandSpawn API. Manages the native instance lifetime, exposes shell and pipe configuration as properties, and provides idiomatic .NET methods for process execution and pipe I/O.

Key responsibilities:
- Wraps `CommandSpawn_Create` / `CommandSpawn_Destroy` with `IDisposable`
- Exposes `Shell`, `EscapementStyle`, `E_PIPE` as typed properties/enums
- `Command(string, string, E_PIPE)` → launches process
- `Join()` → blocks for exit code
- `ReadLine(E_PIPE)`, `ReadToEnd(E_PIPE)`, `WriteLine(string)` → pipe I/O
- `HasData(E_PIPE)` → non-blocking pipe check

Notes: Native library must be discoverable at runtime. On Linux set `LD_LIBRARY_PATH` to include the staged lib lane, or use `in_this_context.sh`.

### `TestCommandSpawn`
File: `Project/JWCCommandSpawn.net/TestCommandSpawn/Program.cs`
Purpose: Basic test/example — demonstrates launching a command and reading stdout.

### `TestNested_feffect`
File: `Project/JWCCommandSpawn.net/TestNested_feffect/Program.cs`
Purpose: Advanced test demonstrating nested/piped command chains with `feffect` terminal effects.

## Architectural Seams
- **Core / Platform split**: `src/CommandSpawn.cpp` contains platform-neutral logic. Platform-specific spawning lives in `src/Platform_Linux/CommandSpawn_Linux.cpp` (fork/exec/pipe) and `src/Platform_Windows/CommandSpawn_Windows.cpp` (CreateProcess). A new platform implementation goes in `src/Platform_<OS>/`.
- **Args abstraction (Linux)**: `src/Platform_Linux/Args.cpp` converts `utf8_string_struct` command strings to `argv`-style arrays for `execv`. Touch this when modifying argument passing on Linux.
- **Export boundary**: `include/JWCCommandSpawn/CommandSpawn.h` is the only public header. All ABI-visible symbols live here. Do not expose C++ types across this boundary — use opaque pointers (`P_INSTANCE`) and primitive/JWCEssentials types.
- **Managed/Native boundary**: `CommandSpawn.cs` P/Invoke wraps the C interface. The native instance handle is an `IntPtr`; lifetime is managed via `Dispose`.

## Extension Points
- **New shell type**: Add a value to `CommandSpawn::Shell`, implement detection in the platform `.cpp` file, and expose a new `CommandSpawn_GetShell_<Name>` export.
- **New escapement style**: Add a value to `CommandSpawn::EscapementStyle` and implement the escaping logic in `CommandSpawn_escapeStringForCommandLine`.
- **New platform**: Add `src/Platform_<OS>/CommandSpawn_<OS>.cpp`, implement the spawn/pipe/wait interface, and wire into CMakeLists.txt with a platform detection block.
- **New managed method**: Add the `DllImport` to `CommandSpawn.cs` and the managed wrapper method alongside it.

## Known Cautions
- **Always call `CommandSpawn_Destroy`**: failure to destroy leaks process handles and pipe file descriptors.
- **Join before Destroy**: calling `Destroy` without `Join` may leave zombie processes on Linux.
- **`ClosePipe` for stdin EOF**: child processes waiting for stdin EOF will hang unless `CommandSpawn_ClosePipe(This, E_PIPE_STDIN)` is called.
- **Single-line `_EXPORT_`**: required for grep-based tooling. Do not split across lines.
- **Escapement mismatch**: setting `EscapementStyle::None` on a shell that adds quoting can cause double-escaping; always test with `CommandSpawn_ToString` first.
- **Managed runtime discovery**: `libJWCCommandSpawn.so` must be on `LD_LIBRARY_PATH` at runtime on Linux. Use `in_this_context.sh` or `newage_source_context.sh` to enter the correct lane environment.

## Generated / Reflected / Derived Files
- **`$NewAge/include/JWCCommandSpawn`**: symlink registered by `configure.sh` to `include/JWCCommandSpawn/`. Source of truth is the repo; do not edit through the workspace symlink.
- **Build artifacts**: `build/`, `bin/`, `obj/` — not tracked. Rebuild via `Dev/build_native.sh` and `Dev/build_managed.sh`.

## Maintenance Notes
- **Refresh policy**: update after adding exports, new shell types, new escapement styles, or changing platform implementations.
- **Path consistency**: always use repo-relative paths.
- **`_EXPORT_` grep-friendliness**: keep all declarations single-line.
