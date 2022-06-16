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

            Console.WriteLine("1) Verify/Install game from Steam");
            Console.WriteLine("2) Patch game files for mod loading (requires an install)");
            Console.WriteLine("3) Run SoupedModFramework");
            bool chosen = false;
            while (!chosen)
            {
                Console.Write("> ");
                string choice = Console.ReadLine();
                switch (choice)
                {
                    case "1":

                        chosen = true;
                        break;
                    case "2":
                        chosen = true;
                        break;
                    case "3":
                        chosen = true;
                        break;
                    default:
                        Console.WriteLine("Invalid option, try again");
                        break;
                }
            }
        }
    }
}