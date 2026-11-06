using System;
using System.Diagnostics;
using System.IO;
using System.Reflection;
using System.Security.Cryptography;
using System.Windows.Forms;

internal static class Launcher
{
    [STAThread]
    private static void Main(string[] args)
    {
        try
        {
            var assembly = Assembly.GetExecutingAssembly();
            string version;
            using (var stream = File.OpenRead(assembly.Location))
            using (var hash = SHA256.Create())
                version = BitConverter.ToString(hash.ComputeHash(stream)).Replace("-", "").Substring(0, 16);
            string folder = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData), "LimbiPrototype", "app", version);
            Directory.CreateDirectory(folder);
            foreach (string name in new[] { "Limbi.ps1", "content.json", "Start-Limbi.vbs" })
            {
                using (Stream resource = assembly.GetManifestResourceStream(name))
                using (var output = File.Create(Path.Combine(folder, name)))
                    resource.CopyTo(output);
            }
            var start = new ProcessStartInfo();
            start.FileName = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.System), "WindowsPowerShell", "v1.0", "powershell.exe");
            start.Arguments = "-NoProfile -STA -ExecutionPolicy Bypass -File \"" + Path.Combine(folder, "Limbi.ps1") + "\"";
            if (Array.IndexOf(args, "-Tray") >= 0) start.Arguments += " -Tray";
            start.UseShellExecute = false;
            start.CreateNoWindow = true;
            start.WorkingDirectory = folder;
            using (var process = Process.Start(start))
            {
                process.WaitForExit();
                if (process.ExitCode != 0)
                    MessageBox.Show("Aplicatia nu a putut porni. Verifica daca Windows PowerShell este permis pe acest calculator.", "Limbi", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }
        catch (Exception error)
        {
            MessageBox.Show("Nu am putut porni Limbi:\n" + error.Message, "Limbi", MessageBoxButtons.OK, MessageBoxIcon.Error);
        }
    }
}
