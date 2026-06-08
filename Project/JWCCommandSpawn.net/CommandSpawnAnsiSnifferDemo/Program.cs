//Polling CommandSpawn Pipes Through AnsiEffectSniffer

using System.Reflection;
using System.Text;
using JWCCommandSpawn.net;
using JWCEssentials;


CommandSpawn spawn = new CommandSpawn();
spawn.SetShell(spawn.GetShell_Bash());

AnsiEffectSniffer stdoutSniffer = new AnsiEffectSniffer();
AnsiEffectSniffer stderrSniffer = new AnsiEffectSniffer();

var stdoutText = new StringBuilder();
var stderrText = new StringBuilder();

stdoutSniffer.Glyph = glyph =>
{
    stdoutText.Append(glyph);
    Console.Write(glyph);
};

stderrSniffer.Glyph = glyph =>
{
    stderrText.Append(glyph);
    Console.Error.Write(glyph);
};

bool ExitNow = false;

spawn.Command(Path.Join(Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location), "ansi_interactive_demo.sh"), 
    CommandSpawn.E_PIPE.E_PIPE_STDOUT | CommandSpawn.E_PIPE.E_PIPE_STDERR);

new Thread(PollingCommandSpawnPipesThroughAnsiEffectSniffer).Start();

spawn.Join();
ExitNow = true;

void PollingCommandSpawnPipesThroughAnsiEffectSniffer()
{
    while (!ExitNow || spawn.HasData()|| spawn.HasData(CommandSpawn.E_PIPE.E_PIPE_STDERR))
    {
        while (spawn.HasData())
        {
            var ch = spawn.ReadByte();
            if (ch is >= 0 and <= 0xFF) stdoutSniffer.ProcessByte((byte)ch);
        }

        while (spawn.HasData(CommandSpawn.E_PIPE.E_PIPE_STDERR))
        {
            var ch = spawn.ReadByte(CommandSpawn.E_PIPE.E_PIPE_STDERR);
            if (ch is >= 0 and <= 0xFF) stderrSniffer.ProcessByte((byte)ch);
        }

        Thread.Sleep(1);
    }
}