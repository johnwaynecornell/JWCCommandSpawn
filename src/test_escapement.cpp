#include "JWCCommandSpawn/CommandSpawn.h"
#include <iostream>
#include <cassert>
#include <string>
#include <vector>

using namespace JWCCommandSpawn;

void test_escapement() {
    P_INSTANCE(CommandSpawn) proc = CommandSpawn_Create();
    
    struct TestCase {
        const char* input;
        CommandSpawn::EscapementStyle style;
        const char* expected_partial;
    };

    std::vector<TestCase> tests = {
        {"simple", CommandSpawn::EscapementStyle_None, "simple"},
        {"with space", CommandSpawn::EscapementStyle_PosixShell, "'with space'"},
        {"with'quote", CommandSpawn::EscapementStyle_PosixShell, "'with'\\''quote'"},
        {"with space", CommandSpawn::EscapementStyle_WindowsCommandLine, "\"with space\""},
        {"with\"quote", CommandSpawn::EscapementStyle_WindowsCommandLine, "\"with\\\"quote\""},
        {"backslash\\", CommandSpawn::EscapementStyle_WindowsCommandLine, "\"backslash\\\\\""},
        {"with space", CommandSpawn::EscapementStyle_CmdExe, "\"with space\""},
        {"with\"quote", CommandSpawn::EscapementStyle_CmdExe, "\"with^\"quote\""},
        {"with'quote", CommandSpawn::EscapementStyle_PowerShell, "'with''quote'"},
        {"", CommandSpawn::EscapementStyle_PosixShell, "''"},
        {"$meta", CommandSpawn::EscapementStyle_PosixShell, "'$meta'"},
    };

    for (const auto& test : tests) {
        CommandSpawn_SetEscapementStyle(proc, test.style);
        // We test via CommandSpawn_ToString which internally calls the helper
        // We need to set a shell for ToString to perform escaping
        CommandSpawn_SetShellExplicit(proc, "test", "test", "-c");
        
        utf8_string_struct ts = CommandSpawn_ToString(proc, test.input);
        std::string ts_str(ts.c_str);
        
        std::cout << "Input: [" << test.input << "] Style: " << test.style << " ToString: [" << ts_str << "]" << std::endl;
        
        if (test.style == CommandSpawn::EscapementStyle_None) {
             assert(ts_str == "test -c " + std::string(test.input));
        } else {
             assert(ts_str.find(test.expected_partial) != std::string::npos);
        }
    }

    // Test Auto behavior on Linux (bash detection)
    CommandSpawn_SetEscapementStyle(proc, CommandSpawn::EscapementStyle_Auto);
    CommandSpawn_SetShellExplicit(proc, "bash", "/bin/bash", "-c");
    utf8_string_struct ts_bash = CommandSpawn_ToString(proc, "echo hello");
    std::cout << "Auto + Bash: [" << ts_bash.c_str << "]" << std::endl;
    assert(std::string(ts_bash.c_str).find("'echo hello'") != std::string::npos);

    CommandSpawn_Destroy(proc);
    std::cout << "All escapement tests passed!" << std::endl;
}

int main() {
    try {
        test_escapement();
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
