//
// Created by jwc on 7/26/24.
//

#include "test.h"

#include <iostream>

using namespace JWCEssentials;
using namespace JWCCommandSpawn;

void shell_io() {
    //CommandSpawn::CommandHandle h;

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
    proc->Command(nullptr, nullptr, (CommandSpawn::E_PIPE)(CommandSpawn::E_PIPE_STDIN | CommandSpawn::E_PIPE_STDOUT | CommandSpawn::E_PIPE_STDERR));

    std::cout << feffect( "fg_yellow(\"sudo -S ls -l\")") << std::endl;
    //proc->WriteLine("sudo -S ls -l");
    proc->WriteLine("ls -l");
    proc->WriteLine("exit");

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

}

CommandSpawn *proc;

utf8_string_struct Exec(utf8_string_struct command, utf8_string_struct input)
{
    utf8_string_struct cmd = command;
    utf8_string_struct forstdin = input;

    // CommandSpawn_Command(proc, cmd, nullptr, (CommandSpawn::E_PIPE) (CommandSpawn::E_PIPE_STDOUT | CommandSpawn::E_PIPE_STDIN));
    // CommandSpawn_WriteString(proc, forstdin);
    // CommandSpawn_ClosePipe(proc, CommandSpawn::E_PIPE_STDIN);

    CommandSpawn_Command(proc, cmd, forstdin, (CommandSpawn::E_PIPE) (CommandSpawn::E_PIPE_STDOUT));

    //CommandSpawn_ClosePipe(proc, CommandSpawn::E_PIPE_STDIN);

    utf8_string_struct s = CommandSpawn_ReadToEnd(proc, CommandSpawn::E_PIPE_STDOUT);

    CommandSpawn_Join(proc);
    return s;
}

void inject() {
    proc = CommandSpawn_Create();

    utf8_string_struct line;

    proc->SetShell(proc->GetShell_Bash());

    Exec("xargs -I{} echo {}", "1\n2\n3\n");
    Exec("xargs -I{} echo {}", "1\n2\n3\n");

    return;

    //proc->Command("xargs -I{} echo {}", nullptr, (CommandSpawn::E_PIPE)(CommandSpawn::E_PIPE_STDIN | CommandSpawn::E_PIPE_STDOUT |CommandSpawn::E_PIPE_STDERR));
    proc->Command("xargs -I{} echo {}", "1\n2\n3\n", (CommandSpawn::E_PIPE)(CommandSpawn::E_PIPE_STDIN | CommandSpawn::E_PIPE_STDOUT |CommandSpawn::E_PIPE_STDERR));

    //line = proc->ReadLine(CommandSpawn::E_PIPE_STDOUT);

    //std::cout << "got " << feffect( "fg_green(\""+line + "\")", nullptr) << std::endl;

    while ((line = proc->ReadLine(CommandSpawn::E_PIPE_STDOUT)) != nullptr) {
        std::cout << feffect( "fg_green(\""+line+"\")") << std::endl;
    }
    proc->Join();

    proc->Command("xargs -I{} echo {}", "1\n2\n3\n", (CommandSpawn::E_PIPE)(CommandSpawn::E_PIPE_STDIN | CommandSpawn::E_PIPE_STDOUT |CommandSpawn::E_PIPE_STDERR));

    //line = proc->ReadLine(CommandSpawn::E_PIPE_STDOUT);

    //std::cout << "got " << feffect( "fg_green(\""+line + "\")", nullptr) << std::endl;

    while ((line = proc->ReadLine(CommandSpawn::E_PIPE_STDOUT)) != nullptr) {
        std::cout << feffect( "fg_green(\""+line+"\")") << std::endl;
    }
    proc->Join();

}

int main() {
    inject();


    return 0;
}
