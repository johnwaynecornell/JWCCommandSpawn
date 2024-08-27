#ifndef SPAWNEDPROCESS_H
#define SPAWNEDPROCESS_H

#include <string>
#include <vector>

#include "JWCEssentials/JWCEssentials.h"

using namespace JWCEssentials;

#ifdef BUILD_SPAWNEDPROCESS
    #define _EXPORT_ __EXPORT__
    #define _CLASSEXPORT_ __CLASSEXPORT__
#else
    #define _EXPORT_ __IMPORT__
    #define _CLASSEXPORT_ __CLASSIMPORT__
#endif

namespace JWCCommandSpawn {
    class _CLASSEXPORT_ CommandSpawn {
    public:
        enum E_PIPE
        {
            E_PIPE_NONE=0,
            E_PIPE_STDOUT=1,
            E_PIPE_STDERR=2,
            E_PIPE_STDIN=4
        };

        bool END[5] = {};

        struct CommandHandle
        {
            CommandSpawn *myCommandSpawn;
            _CLASSEXPORT_ CommandHandle();
            _CLASSEXPORT_ CommandHandle(CommandSpawn *myCommandSpawn);

            _CLASSEXPORT_ CommandHandle & operator=(CommandHandle &&other);
            _CLASSEXPORT_ CommandHandle & operator=(CommandSpawn *myCommandSpawn);
            _CLASSEXPORT_ ~CommandHandle();

            _CLASSEXPORT_ operator bool();
            _CLASSEXPORT_ void release();
        };

        struct Shell {
            utf8_string_struct shell = nullptr;
            utf8_string_struct shell_switch = nullptr;

            _CLASSEXPORT_ Shell(Shell &other);
            _CLASSEXPORT_ Shell(Shell &&other) noexcept;

            _CLASSEXPORT_ Shell(utf8_string_struct shell, utf8_string_struct shell_switch);

            _CLASSEXPORT_ Shell & operator = (Shell &other);
            _CLASSEXPORT_ Shell & operator = (Shell &&other) noexcept;

        };

        long last_return = 0;

        Shell shell = {nullptr, nullptr};

        void ClearShell() {
            shell = {nullptr, nullptr};
        }

        CommandSpawn();

        virtual ~CommandSpawn();

        virtual Shell GetShell_Defaultl() = 0;
        virtual Shell GetShell_Bash() = 0;
        virtual Shell GetShell_Python();

        virtual bool HasShell(Shell shell) = 0;

        virtual void SetShell(Shell shell);
        virtual void SetShellExplicit(utf8_string_struct shell, utf8_string_struct shell_switch);
        virtual long Command(utf8_string_struct command, E_PIPE pipes) = 0;

        virtual void Close() = 0;
        virtual long Join() = 0;

        virtual utf8_string_struct ToString(utf8_string_struct command);

        virtual bool HasData(E_PIPE targ) = 0;
        virtual int ReadByte(E_PIPE targ) = 0;
        virtual utf8_string_struct ReadLine(E_PIPE targ);
        virtual utf8_string_struct ReadToEnd(E_PIPE targ);
        virtual utf8_string_struct ReadAll(E_PIPE targ);

        virtual void WriteByte(char byte) = 0;
        virtual void WriteString(utf8_string_struct string);
        virtual void WriteLine(utf8_string_struct line);
        virtual void Flush() = 0;
    };

    utf8_string_struct_array CStyle_ParseByWhitespace(const utf8_string_struct& command);

    _EXPORT_ CommandSpawn *CommandSpawn_Create();
    _EXPORT_ void CommandSpawn_Destroy(CommandSpawn *This);

    _EXPORT_ CommandSpawn::Shell CommandSpawn_GetShell_Defaultl(CommandSpawn *This);
    _EXPORT_ CommandSpawn::Shell CommandSpawn_GetShell_Bash(CommandSpawn *This);
    _EXPORT_ CommandSpawn::Shell CommandSpawn_GetShell_Python(CommandSpawn *This);

    _EXPORT_ bool CommandSpawn_HasShell(CommandSpawn *This, CommandSpawn::Shell shell);

    _EXPORT_ void SetCommandSpawn_SetShell(CommandSpawn *This, CommandSpawn::Shell shell);
    _EXPORT_ void SetCommandSpawn_SetShellExplicit(CommandSpawn *This, utf8_string_struct shell, utf8_string_struct shell_switch);
    _EXPORT_ bool CommandSpawn_Command(CommandSpawn *This, utf8_string_struct command, CommandSpawn::E_PIPE pipes);

    _EXPORT_ long CommandSpawn_Join(CommandSpawn * This);

    _EXPORT_ utf8_string_struct CommandSpawn_ToString(CommandSpawn * This, utf8_string_struct command);

    _EXPORT_ bool CommandSpawn_HasData(CommandSpawn * This, CommandSpawn::E_PIPE targ);
    _EXPORT_ int CommandSpawn_ReadByte(CommandSpawn * This, CommandSpawn::E_PIPE targ);
    _EXPORT_ utf8_string_struct CommandSpawn_ReadLine(CommandSpawn * This, CommandSpawn::E_PIPE targ);
    _EXPORT_ utf8_string_struct CommandSpawn_ReadToEnd(CommandSpawn * This, CommandSpawn::E_PIPE targ);

    _EXPORT_ void CommandSpawn_WriteByte(CommandSpawn *This, char byte);
    _EXPORT_ void CommandSpawn_WriteString(CommandSpawn * This, utf8_string_struct string);
    _EXPORT_ void CommandSpawn_WriteLine(CommandSpawn * This, utf8_string_struct line);
}

#undef _EXPORT_
#undef  _CLASSEXPORT_

#endif