namespace SoupedModFramework
{
    using System;
    using System.Windows.Forms;
    using Microsoft.Web.WebView2.Core;
    using Microsoft.Web.WebView2.WinForms;

    public class Launcher
    {
        [STAThread]
        public static void Main(string[] args)
        {
            Console.WriteLine("Loading SMF Launcher...");

            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            Application.Run(new WebView());
        }
    }
}