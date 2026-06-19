using JWCCommandSpawn.net;
using JWCEssentials.net;

public class Program
{
    public static void Main(string[] args)
    {   
        Essentials.EnableTerminalEffects();

        CommandSpawn command = new CommandSpawn();
        command.SetShell(command.GetShell_Bash());

        command.EscapementStyle = CommandSpawn.E_EscapementStyle.None;
        string input = "feffect -e 'fg_blue.bg_bright_white(\"Hello from the inside\")'";
        string cmd_text = CommandSpawn.escapeStringForCommandLine(input, CommandSpawn.E_EscapementStyle.Auto);

        Console.WriteLine(command.ToString(cmd_text));
        command.Command(cmd_text, CommandSpawn.E_PIPE.E_PIPE_STDOUT | CommandSpawn.E_PIPE.E_PIPE_STDERR);

        string s = command.ReadToEnd(CommandSpawn.E_PIPE.E_PIPE_STDERR);
        if (s != "")
        {
            s = Essentials.feffect($"fg_red(\"{s}\")");
            Console.WriteLine(s);
        }

        s = command.ReadToEnd(CommandSpawn.E_PIPE.E_PIPE_STDOUT);
        
        Console.WriteLine(s);
        
        command.Join();
    }
}