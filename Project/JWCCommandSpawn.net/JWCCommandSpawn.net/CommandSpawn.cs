using System.Runtime.InteropServices;
using System.Text;
using JWCEssentials.net;

namespace JWCCommandSpawn.net;

public class CommandSpawn
{
    
    [Flags]
    public enum E_PIPE
        {
            E_PIPE_NONE=0,
            E_PIPE_STDOUT=1,
            E_PIPE_STDERR=2,
            E_PIPE_STDIN=4
        };

    public struct Shell
    {
        public utf8_string_struct name;
        public utf8_string_struct shell;
        public utf8_string_struct shell_switch;
    };

    public class DllImports
    {
        [DllImport("JWCCommandSpawn")]
        public static extern IntPtr CommandSpawn_Create();
        [DllImport("JWCCommandSpawn")]
        public static extern void CommandSpawn_Destroy(IntPtr This);

        [DllImport("JWCCommandSpawn")]
        public static extern CommandSpawn.Shell CommandSpawn_GetShell_Defaultl(IntPtr This);
        [DllImport("JWCCommandSpawn")]
        public static extern CommandSpawn.Shell CommandSpawn_GetShell_Bash(IntPtr This);
        [DllImport("JWCCommandSpawn")]
        public static extern CommandSpawn.Shell CommandSpawn_GetShell_Python(IntPtr This);

        [DllImport("JWCCommandSpawn")]
        public static extern bool CommandSpawn_HasShell(IntPtr This, ref CommandSpawn.Shell shell);

        [DllImport("JWCCommandSpawn")]
        public static extern void CommandSpawn_SetShell(IntPtr This, ref CommandSpawn.Shell shell);
        [DllImport("JWCCommandSpawn")]
        public static extern void CommandSpawn_SetShellExplicit(IntPtr
            This, ref utf8_string_struct name , ref utf8_string_struct shell, ref utf8_string_struct shell_switch);
        [DllImport("JWCCommandSpawn")]
        public static extern bool CommandSpawn_Command(IntPtr
            This, ref utf8_string_struct command, ref utf8_string_struct for_stdin, CommandSpawn.E_PIPE pipes);

        [DllImport("JWCCommandSpawn")]
        public static extern long CommandSpawn_Join(IntPtr This);

        [DllImport("JWCCommandSpawn")]
        public static extern void CommandSpawn_ClosePipe(IntPtr This, CommandSpawn.E_PIPE pipes);
        
        [DllImport("JWCCommandSpawn")]
        public static extern utf8_string_struct CommandSpawn_ToString(IntPtr This, ref utf8_string_struct command);

        [DllImport("JWCCommandSpawn")]
        public static extern bool CommandSpawn_HasData(IntPtr This, CommandSpawn.E_PIPE targ);
        [DllImport("JWCCommandSpawn")]
        public static extern int CommandSpawn_ReadByte(IntPtr This, CommandSpawn.E_PIPE targ);
        [DllImport("JWCCommandSpawn")]
        public static extern utf8_string_struct CommandSpawn_ReadLine(IntPtr This, CommandSpawn.E_PIPE targ);
        [DllImport("JWCCommandSpawn")]
        public static extern utf8_string_struct CommandSpawn_ReadToEnd(IntPtr This, CommandSpawn.E_PIPE targ);

        [DllImport("JWCCommandSpawn")]
        public static extern void CommandSpawn_WriteByte(IntPtr This, char _byte);
        [DllImport("JWCCommandSpawn")]
        public static extern void CommandSpawn_WriteString(IntPtr This, ref utf8_string_struct _string);
        [DllImport("JWCCommandSpawn")]
        public static extern void CommandSpawn_WriteLine(IntPtr This, ref utf8_string_struct line);
    }

    public IntPtr Handle;

    public CommandSpawn()
    {
        Handle = DllImports.CommandSpawn_Create();
    }

    ~CommandSpawn()
    {
        DllImports.CommandSpawn_Destroy(Handle);
    }

    public Shell GetShell_Bash()
    {
        return CommandSpawn.DllImports.CommandSpawn_GetShell_Bash(Handle);
    }

    public string Name = "";
    public void SetShell(Shell shell)
    {
        Name = shell.name;
        DllImports.CommandSpawn_SetShell(Handle, ref shell);
    }

    public void Command(string command, E_PIPE pipes)
    {
        utf8_string_struct cmd = command;
        utf8_string_struct forstdin = null;

        DllImports.CommandSpawn_Command(Handle, ref cmd, ref forstdin, pipes);
    }

    public string Exec(string command)
    {
        utf8_string_struct cmd = command;
        utf8_string_struct forstdin = null;

        DllImports.CommandSpawn_Command(Handle, ref cmd, ref forstdin, E_PIPE.E_PIPE_STDOUT);
        
        string s = ReadToEnd();
        DllImports.CommandSpawn_Join(Handle);
        return s;
    }
    
    public string PipeCommand(string command)
    {
        utf8_string_struct cmd = command;
        utf8_string_struct forstdin = null;

        WriteString(command+"\n");
        
        string s = ReadToEnd();
        
        return s;
    }
    
    /*
    public string BashPipeCommand(string command)
    {
        WriteString($"({command}; echo \"***OK***\")\n");

        StringBuilder s = new StringBuilder();

        bool c;
        do
        {

            string l = ReadLine();
            if (l == null) throw new Exception("expected OK prompt");
            c = true;
            if (l.EndsWith("***OK***"))
            {
                c = false;
                l = l.Substring(0, l.Length - 8);
            }

            s.AppendLine(l);

        } while (c);

        return s.ToString();
    }
    */
    
    public string Exec(string command, string input)
    {
        utf8_string_struct cmd = command;
        utf8_string_struct forstdin = input;

        DllImports.CommandSpawn_Command(Handle, ref cmd, ref forstdin, E_PIPE.E_PIPE_STDOUT);

        //DllImports.CommandSpawn_ClosePipe(Handle, E_PIPE.E_PIPE_STDIN);
        
        string s = ReadToEnd();
        DllImports.CommandSpawn_Join(Handle);
        return s;
    }
    
    public void Join()
    {
        DllImports.CommandSpawn_Join(Handle);
    }

    public void WriteString(string input)
    {
        utf8_string_struct i = input;
        DllImports.CommandSpawn_WriteString(Handle, ref i);
    }
    public string ReadToEnd(E_PIPE pipe = E_PIPE.E_PIPE_STDOUT)
    {
        return DllImports.CommandSpawn_ReadToEnd(Handle, pipe);
    }
    public string ReadLine(E_PIPE pipe = E_PIPE.E_PIPE_STDOUT)
    {
        return DllImports.CommandSpawn_ReadLine(Handle, pipe);
    }
    
    public CommandPipe CreateCommandPipe()
    {
        Func<string, string> _wrapCommand = null;
        CommandPipe.parse_delegate _parseOutputLine = null;

        if (Name == "bash")
        {
            _wrapCommand = BashWrapCommand;
            _parseOutputLine = BashParseOutputLine;
        }
        
        return new CommandPipe(this, _wrapCommand, _parseOutputLine);
    }

    private string BashWrapCommand(string command)
    {
        return $"({command}; echo \"***OK*** EXIT:$?\")";
    }

    private string? BashParseOutputLine(string line, CommandPipe pipe)
    {
        if (line.Contains("***OK***"))
        {
            int exitIndex = line.IndexOf("EXIT:");
            if (exitIndex != -1 && int.TryParse(line.Substring(exitIndex + 5), out int exitCode) && exitCode != 0)
            {
                throw new Exception($"Command failed with exit code {exitCode}.");
            }

            pipe.Sentinel();
            return line.Substring(0, line.IndexOf("***OK***")).TrimEnd();
        }

        return line.TrimEnd();
    }
}

public class CommandPipe
{
    public delegate string? parse_delegate(string input, CommandPipe pipe);
    
    private readonly CommandSpawn _commandSpawn;
    private readonly Func<string, string> _wrapCommand;
    private readonly parse_delegate _parseOutputLine;
    private bool _sentinelEncountered;
    public CommandPipe(
        CommandSpawn commandSpawn,
        Func<string, string> wrapCommand,
        parse_delegate parseOutputLine)
    {
        _commandSpawn = commandSpawn;
        _wrapCommand = wrapCommand;
        _parseOutputLine = parseOutputLine;
        _sentinelEncountered = false;
    }

    public void Sentinel()
    {
        _sentinelEncountered = true;
    }

    public void SendCommand(string command)
    {
        string wrappedCommand = _wrapCommand(command);
        _commandSpawn.WriteString(wrappedCommand + "\n");
        _sentinelEncountered = false;
    }

    public string? ReadOutputLine()
    {
        if (_sentinelEncountered) return null;

        string? line = _commandSpawn.ReadLine();
        if (line == null) throw new Exception("Unexpected EOF before sentinel.");

        string? parsedLine = _parseOutputLine(line, this);
        if (parsedLine == null)
        {
            _sentinelEncountered = true;
            return null;
        }

        return parsedLine;
    }

    public string Exec(string command)
    {
        StringBuilder b = new StringBuilder();
        SendCommand(command);

        string line;
        while ((line = ReadOutputLine()) != null)
        {
            b.AppendLine(line);
        }

        return b.ToString();
    }
}
