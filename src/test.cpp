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
    //proc->SetShell(proc->GetShell_Defaultl());

    utf8_string_struct line;

    proc->SetShell(proc->GetShell_Bash());
/*
    proc->Command("ls -a", CommandSpawn::E_PIPE_STDOUT);
    while ((line = proc->ReadLine(CommandSpawn::E_PIPE_STDOUT)) != nullptr) {
        std::cout << "got " << line << std::endl;

    }
    proc->Join();
*/
    proc->Command(nullptr, (CommandSpawn::E_PIPE)(CommandSpawn::E_PIPE_STDIN | CommandSpawn::E_PIPE_STDOUT | CommandSpawn::E_PIPE_STDERR));

    std::cout << feffect( "fg_yellow(\"sudo -S ls -l\")") << std::endl;
    proc->WriteLine("sudo -S ls -l");

    bool C;
    do {
        C = true;
        if (proc->HasData(CommandSpawn::E_PIPE_STDERR)) {
            utf8_string_struct err_string = proc->ReadAll(CommandSpawn::E_PIPE_STDERR);

            proc->WriteLine(""); //password

        } else if (proc->HasData(CommandSpawn::E_PIPE_STDOUT)) C = false;

    } while (C);

    //line = proc->ReadLine(CommandSpawn::E_PIPE_STDOUT);

    //std::cout << "got " << feffect( "fg_green(\""+line + "\")", nullptr) << std::endl;

    while ((line = proc->ReadLine(CommandSpawn::E_PIPE_STDOUT)) != nullptr) {
        std::cout << feffect( "fg_green(\""+line+"\")") << std::endl;
    }
    proc->Join();



    return 0;
}
