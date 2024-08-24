//
// Created by jwc on 7/26/24.
//

#include "test.h"

#include <iostream>

using namespace JWCEssentials;
using namespace JWCCommandSpawn;

int main() {
    CommandSpawn::CommandHandle h;

    CommandSpawn *proc = CommandSpawn_Create();
    proc->SetShell(proc->GetShell_Defaultl());

    h = proc->Command("dir /s", CommandSpawn::E_PIPE_NONE);
    h = proc->Command("dir /s", CommandSpawn::E_PIPE_STDOUT);

    utf8_string_struct line;
    while ((line = proc->ReadLine(CommandSpawn::E_PIPE_STDOUT)) != nullptr) {
        std::cout << "got " << line << std::endl;

    }

    h.release();

    proc->SetShell(proc->GetShell_Bash());

    h = proc->Command("ls -a", CommandSpawn::E_PIPE_NONE);
    h = proc->Command("ls - a", CommandSpawn::E_PIPE_STDOUT);


    return 0;
}
