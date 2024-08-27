#include "JWCCommandSpawn/CommandSpawn.h"
#include <Windows.h>
#include <iostream>
#include <vector>
#include <sstream>

using namespace JWCEssentials;
namespace JWCCommandSpawn
{

void CommandSpawn_VerifyStringContained(const char *string, size_t string_bytes, const char *msg);

utf8_string_struct LastErrorString()
{
    LPVOID lpMsgBuf;
    DWORD dw = GetLastError();
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                  NULL, dw, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgBuf, 0, NULL);

    utf8_string_struct R = (LPTSTR) lpMsgBuf;
    //TRACE(_T("::executeCommandLine() failed at CreateProcess()\nCommand=%s\nMessage=%s\n\n"), cmdLine, strError);

    // Free resources created by the system
    LocalFree(lpMsgBuf);

    return R;
}



class CommandSpawn_Windows : public CommandSpawn {
private:
    HANDLE g_hChildStd_IN_Rd = NULL;
    HANDLE g_hChildStd_IN_Wr = NULL;

    HANDLE g_hChildStd_OUT_Rd = NULL;
    HANDLE g_hChildStd_OUT_Wr = NULL;

    HANDLE g_hChildStd_ERR_Rd = NULL;
    HANDLE g_hChildStd_ERR_Wr = NULL;

    PROCESS_INFORMATION pi;

public:
    CommandSpawn_Windows() : CommandSpawn() {
    }

    ~CommandSpawn_Windows() override {
        Join();
    }

    long Join() override
    {
        WaitForSingleObject(pi.hProcess, INFINITE);

        DWORD exitCode;
        // Get the exit code.
        GetExitCodeProcess(pi.hProcess, &exitCode);

        Close();
        return exitCode;
    }

    Shell GetShell_Defaultl() override {
        return { "cmd", "/c" };
    }

    Shell GetShell_Bash() override {
        return { "bash", "-c" };
    }

    void DiscardHandle(HANDLE &H)
    {
        CloseHandle(H);
        H = nullptr;
    }

    long Command(utf8_string_struct command, E_PIPE pipes) override{
        STARTUPINFO si;
        SECURITY_ATTRIBUTES sa;

        // Set the bInheritHandle flag so pipe handles are inherited
        sa.nLength = sizeof(SECURITY_ATTRIBUTES);
        sa.bInheritHandle = TRUE;
        sa.lpSecurityDescriptor = NULL;

        ZeroMemory(&si, sizeof(STARTUPINFO));
        si.cb = sizeof(STARTUPINFO);
        si.dwFlags |= STARTF_USESTDHANDLES;

        if (pipes & E_PIPE_STDOUT) {
            // Create a pipe for the child process's STDOUT
            if (!CreatePipe(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr, &sa, 0))
                throw std::runtime_error("Stdout pipe creation failed");

            // Ensure the read handle to the pipe for STDOUT is not inherited
            if (!SetHandleInformation(g_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0))
                throw std::runtime_error("Stdout SetHandleInformation failed");

            si.hStdOutput = g_hChildStd_OUT_Wr;
        } else {
            si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
        }

        if (pipes & E_PIPE_STDERR) {
            // Create a pipe for the child process's STDERR
            if (!CreatePipe(&g_hChildStd_ERR_Rd, &g_hChildStd_ERR_Wr, &sa, 0))
                throw std::runtime_error("Stderr pipe creation failed");

            // Ensure the read handle to the pipe for STDERR is not inherited
            if (!SetHandleInformation(g_hChildStd_ERR_Rd, HANDLE_FLAG_INHERIT, 0))
                throw std::runtime_error("Stderr SetHandleInformation failed");

            si.hStdError = g_hChildStd_ERR_Wr;
        } else {
            si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
        }

        if (pipes & E_PIPE_STDIN) {
            // Create a pipe for the child process's STDIN
            if (!CreatePipe(&g_hChildStd_IN_Rd, &g_hChildStd_IN_Wr, &sa, 0))
                throw std::runtime_error("Stdin pipe creation failed");

            // Ensure the write handle to the pipe for STDIN is not inherited
            if (!SetHandleInformation(g_hChildStd_IN_Wr, HANDLE_FLAG_INHERIT, 0))
                throw std::runtime_error("Stdin SetHandleInformation failed");

            si.hStdInput = g_hChildStd_IN_Rd;
        } else {
            si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
        }

        // Set up members of the PROCESS_INFORMATION structure
        ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));

        utf8_string_struct command_str = ToString(command);

        bool rc = true;
        // Create the child process
        if (!CreateProcess(NULL,
                           command_str,  // command line
                           NULL,                               // process security attributes
                           NULL,                               // primary thread security attributes
                           TRUE,                               // handles are inherited
                           0,                                  // creation flags
                           NULL,                               // use parent's environment
                           NULL,                               // use parent's current directory
                           &si,                                // STARTUPINFO pointer
                           &pi))                               // receives PROCESS_INFORMATION
        {
            std::cerr << command_str << " not found";
            std::cerr << LastErrorString();

            rc = false;
        }

        // Close handles to the stdin and stdout pipes no longer needed by the parent process
        if (pipes & E_PIPE_STDIN) DiscardHandle(g_hChildStd_IN_Rd);
        if (pipes & E_PIPE_STDOUT) DiscardHandle(g_hChildStd_OUT_Wr);
        if (pipes & E_PIPE_STDERR) DiscardHandle(g_hChildStd_ERR_Wr);

        return rc ? 0 : -1;
    }

    bool HasShell(Shell shell) override {
        return resolve(shell.shell);
    }

    static bool FileExists(const std::string& filename)
    {
        WIN32_FIND_DATAA fd = {0};
        HANDLE hFound = FindFirstFileA(filename.c_str(), &fd);
        bool retval = hFound != INVALID_HANDLE_VALUE;
        FindClose(hFound);

        return retval;
    }

    static utf8_string_struct resolveExecutablePath(const std::string &executable) {

        // Get the PATH environment variable
        char *pathEnv;
        size_t env_size;

        _dupenv_s(&pathEnv, &env_size, "PATH");

        //const char *pathEnv = std::getenv("PATH");
        if (!pathEnv) {
            return "";
        }

        std::string pathStr(pathEnv);
        std::istringstream pathStream(pathStr);
        std::string pathDir;
        std::vector<std::string> paths;

        // Split the PATH environment variable into individual directories
        while (std::getline(pathStream, pathDir, ';')) {
            paths.push_back(pathDir);
        }

        // Search for the executable in each directory
        for (const auto &dir : paths) {
            std::string fullPath = dir + "\\" + executable;
            if (FileExists(fullPath)) {
                free(pathEnv);
                return fullPath.c_str();
            }
        }

        free(pathEnv);
        // Executable not found
        return nullptr;
    }

    static utf8_string_struct resolve(utf8_string_struct path) {
        bool qualified = false;
        bool escape = false;

        for (int i=0; i < path.length; i++) {
            if (!escape) {
                if (path[i] == '\\') qualified = true;
                //if (path[i] == '/') qualified = true;
            } else escape = false;
        }

        if (!qualified) {
            utf8_string_struct qpath = resolveExecutablePath(path.c_str);
            return qpath;
        } else if (FileExists(path.c_str)) {
            return path;
        }

        return nullptr;
    }

    void Close() override {
        if (g_hChildStd_IN_Rd) DiscardHandle(g_hChildStd_IN_Rd);
        if (g_hChildStd_IN_Wr) DiscardHandle(g_hChildStd_IN_Wr);

        if (g_hChildStd_OUT_Rd) DiscardHandle(g_hChildStd_OUT_Rd);
        if (g_hChildStd_OUT_Wr) DiscardHandle(g_hChildStd_OUT_Wr);

        if (g_hChildStd_ERR_Rd) DiscardHandle(g_hChildStd_ERR_Rd);
        if (g_hChildStd_ERR_Wr) DiscardHandle(g_hChildStd_ERR_Wr);

        // Close handles to the child process and its primary thread
        DiscardHandle(pi.hProcess);
        DiscardHandle(pi.hThread);
    }

    bool HasData(E_PIPE targ) override {
        HANDLE pipe;

        if (targ == E_PIPE_STDOUT) pipe = g_hChildStd_OUT_Rd;
        else if (targ == E_PIPE_STDERR) pipe = g_hChildStd_ERR_Rd;
        else throw std::runtime_error("Invalid output stream selector");

        DWORD bytesAvailable;

        if (PeekNamedPipe(pipe, NULL, 0, NULL, &bytesAvailable, NULL)) {
            if (bytesAvailable > 0) return true;
        }

        return false;
    }

    int ReadByte(E_PIPE targ) override {
        char ch;
        DWORD dwRead;

        HANDLE pipe;

        if (targ == E_PIPE_STDOUT) pipe = g_hChildStd_OUT_Rd;
        else if (targ == E_PIPE_STDERR) pipe = g_hChildStd_ERR_Rd;
        else throw std::runtime_error("Invalid output stream selector");

        if (!ReadFile(pipe, &ch, 1, &dwRead, NULL) || dwRead == 0)
            return EOF;
        return ch;
    }

    void WriteByte(char byte) override {
        DWORD dwWritten;
        if (!WriteFile(g_hChildStd_IN_Wr, &byte, 1, &dwWritten, NULL))
            throw std::runtime_error("WriteByte failed");
    }

};

CommandSpawn *CommandSpawn_Create() {
    return new CommandSpawn_Windows();
}}