// The public EXE contains the same ZIP used by the in-app updater. It runs
// on the .NET Framework shipped with the supported versions of Windows.
using System;
using System.Diagnostics;
using System.Drawing;
using System.IO;
using System.IO.Compression;
using System.Reflection;
using System.Security.Cryptography;
using System.Threading.Tasks;
using System.Windows.Forms;

[assembly: AssemblyTitle("Open Potato Mapper")]
[assembly: AssemblyProduct("Potato Mapper")]

internal static class PortableDownload
{
    const string Launcher = "PotatoMapper.exe";

    static string GetLauncher(string folder, ZipArchive package, string name)
    {
        string launcher = Path.Combine(folder, Launcher);
        var entry = package.GetEntry(name + "/" + Launcher);
        if (entry == null)
            throw new InvalidDataException("This download is incomplete. Download a fresh copy of Potato Mapper.");
        if (File.Exists(launcher) && File.Exists(Path.Combine(folder, "installation.txt")) &&
            File.ReadAllText(Path.Combine(folder, "installation.txt")) == "PotatoMapper/1\n")
        {
            using (var sha = SHA256.Create())
            using (var bundled = entry.Open())
            using (var present = File.OpenRead(launcher))
                if (BitConverter.ToString(sha.ComputeHash(bundled)) == BitConverter.ToString(sha.ComputeHash(present)))
                    return launcher;
        }
        throw new IOException("That folder already contains other files or a different launcher. " +
            "Open the existing app from its folder, or move this download to another location and run it there.\n\n" + folder);
    }

    static string Unpack()
    {
        var assembly = Assembly.GetExecutingAssembly();
        string parent = Path.GetDirectoryName(assembly.Location);
        string name = "PotatoMapper-" + assembly.GetName().Version.ToString(3) + "-windows-x64";
        string destination = Path.Combine(parent, name);
        using (var stream = assembly.GetManifestResourceStream("PortablePackage.zip"))
        using (var package = new ZipArchive(stream, ZipArchiveMode.Read))
        {
            if (Directory.Exists(destination))
                return GetLauncher(destination, package, name);

            // The public folder appears only after the complete payload has
            // been written. Concurrent opens can share the winning copy.
            string stage = Path.Combine(parent, ".potato-unpack-" + Guid.NewGuid().ToString("N"));
            Directory.CreateDirectory(stage);
            try
            {
                package.ExtractToDirectory(stage);
                string stagedApp = Path.Combine(stage, name);
                GetLauncher(stagedApp, package, name);
                try { Directory.Move(stagedApp, destination); }
                catch (IOException) { if (!Directory.Exists(destination)) throw; }
                return GetLauncher(destination, package, name);
            }
            finally
            {
                Directory.Delete(stage, true);
            }
        }
    }

    [STAThread]
    static int Main(string[] args)
    {
        bool extractOnly = Array.IndexOf(args, "--extract-only") >= 0;
        if (extractOnly)
        {
            try { Unpack(); return 0; }
            catch (Exception e) { Console.Error.WriteLine(e.Message); return 1; }
        }
        Application.EnableVisualStyles();
        Application.SetCompatibleTextRenderingDefault(false);
        int result = 0;
        using (var window = new Form())
        {
            window.Text = "Opening Potato Mapper";
            window.Icon = Icon.ExtractAssociatedIcon(Application.ExecutablePath);
            window.ClientSize = new Size(398, 84);
            window.FormBorderStyle = FormBorderStyle.FixedDialog;
            window.StartPosition = FormStartPosition.CenterScreen;
            window.ControlBox = false;
            window.Controls.Add(new Label { Text = "Preparing the app folder beside this download…",
                AutoSize = true, Left = 18, Top = 16 });
            window.Controls.Add(new ProgressBar { Left = 18, Top = 46, Width = 362, Height = 14,
                Style = ProgressBarStyle.Marquee });
            window.Shown += async delegate
            {
                try
                {
                    string launcher = await Task.Run((Func<string>)Unpack);
                    using (Process.Start(new ProcessStartInfo(launcher) {
                        WorkingDirectory = Path.GetDirectoryName(launcher), UseShellExecute = false })) { }
                }
                catch (Exception e)
                {
                    result = 1;
                    MessageBox.Show(window, e.Message + "\n\nUse a writable Windows folder with enough space for the complete app.",
                        "Potato Mapper could not open", MessageBoxButtons.OK, MessageBoxIcon.Error);
                }
                window.Close();
            };
            Application.Run(window);
        }
        return result;
    }
}
