using JWCCommandSpawn.net;
using JWCEssentials.net;

public class Program
{
    public static void Main(string[] args)
    {
        CommandSpawn command = new CommandSpawn();
        command.SetShell(command.GetShell_Bash());

        command.Command("ls", CommandSpawn.E_PIPE.E_PIPE_STDOUT | CommandSpawn.E_PIPE.E_PIPE_STDERR);

        string s = command.ReadToEnd(CommandSpawn.E_PIPE.E_PIPE_STDERR);
        s = Essentials.feffect($"fg_red(\"{s}\")");
        Console.WriteLine(s);

        s = command.ReadToEnd(CommandSpawn.E_PIPE.E_PIPE_STDOUT);

        Console.WriteLine(s);
    }
}