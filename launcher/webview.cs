namespace SoupedModFramework
{
    using System;
    using System.Drawing;
    using System.Windows.Forms;
    using Microsoft.Web.WebView2.WinForms;

    public class WebView : Form
    {
        private WebView2 view = null;

        public WebView()
        {
            InitializeComponent();
            this.Resize += new System.EventHandler(this.OnResize);
            InitializeAsync();
        }

        private void InitializeComponent()
        {
            this.view = new WebView2();

            this.view.AccessibleName = "webview";
            this.view.Location = new Point(0, 0);
            this.view.Size = new Size(400, 400);
            this.view.Source = new Uri("https://souped.dev", UriKind.Absolute);

            this.Name = "SMF Launcher";

            this.Controls.Add(this.view);
        }

        private async void InitializeAsync()
        {
            await this.view.EnsureCoreWebView2Async(null);
        }

        private void OnResize(object sender, EventArgs e)
        {
            this.view.Size = this.ClientSize - new Size(this.view.Location);
        }
    }
}