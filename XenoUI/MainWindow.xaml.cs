using System.IO;
using System.Text;
using System.Text.RegularExpressions;
using System.Windows;
using System.Windows.Input;
using System.Windows.Media.Imaging;
using System.Windows.Threading;
using Microsoft.Win32;

namespace XenoUI
{
	public partial class MainWindow : Window
	{
		public readonly ClientsWindow _clientsWindow = new();
		private readonly ScriptsWindow _scriptsWindow;
		private readonly DispatcherTimer _timer;
		private string _lastContent;

		public MainWindow()
		{
			InitializeComponent();
			_scriptsWindow = new ScriptsWindow(this);
			Icon = BitmapFrame.Create(new Uri("pack://application:,,,/Resources/Images/icon.ico"));
			InitializeWebView2();

			_timer = new DispatcherTimer { Interval = TimeSpan.FromSeconds(3) };
			_timer.Tick += SaveChanges;
			_timer.Start();
		}

		private async void SaveChanges(object sender, EventArgs e)
		{
			string content = await GetScriptContent();
			if (_lastContent != content)
			{
				File.WriteAllText(Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "bin", "editor.lua"), content);
				_lastContent = content;
			}
		}

		private async void InitializeWebView2()
		{
			try
			{
				await script_editor.EnsureCoreWebView2Async(null);

				string bin = Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "bin");
				string indexPath = Path.Combine(bin, "Monaco", "index.html");
				string tab = Path.Combine(bin, "editor.lua");

				Directory.CreateDirectory(bin);
				if (!File.Exists(tab)) File.WriteAllText(tab, "print(\"Hello, World!\")\n-- Made by .rizve on Discord (https://rizve.us.to)");

				if (!File.Exists(indexPath)) throw new FileNotFoundException("Could not load the Monaco");

				script_editor.Source = new Uri(indexPath);
				await LoadWebView();
				await SetScriptContent(await File.ReadAllTextAsync(tab));
			}
			catch (Exception ex)
			{
				ShowError($"Error initializing WebView2: {ex.Message}");
			}
		}

		private async Task LoadWebView()
		{
			var tcs = new TaskCompletionSource<bool>();
			script_editor.CoreWebView2.NavigationCompleted += (sender, args) =>
			{
				if (args.IsSuccess) tcs.SetResult(true);
				else tcs.SetException(new Exception($"Navigation failed with error code: {args.WebErrorStatus}"));
			};
			await tcs.Task;
		}

		private async Task<string> GetScriptContent()
		{
			string textContent = await script_editor.CoreWebView2.ExecuteScriptAsync("getText()");
			if (textContent.StartsWith("\"") && textContent.EndsWith("\""))
			{
				textContent = textContent[1..^1];
			}

			return Regex.Unescape(textContent);
		}

		private async Task SetScriptContent(string content)
		{
			string escapedContent = EscapeForScript(content);
			await script_editor.CoreWebView2.ExecuteScriptAsync($"setText(\"{escapedContent}\")");
		}

		private static string EscapeForScript(string content) =>
			content.Replace("\\", "\\\\").Replace("\"", "\\\"").Replace("\n", "\\n").Replace("\r", "\\r");

		private static void ShowError(string message)
		{
			MessageBox.Show(message, "Error", MessageBoxButton.OK, MessageBoxImage.Error);
		}

		private void buttonMinimize_Click(object sender, RoutedEventArgs e) => WindowState = WindowState.Minimized;

		private void buttonMaximize_Click(object sender, RoutedEventArgs e)
		{
			WindowState = WindowState == WindowState.Maximized ? WindowState.Normal : WindowState.Maximized;
			maximizeImage.Source = new BitmapImage(new Uri(WindowState == WindowState.Maximized ? "pack://application:,,,/Resources/Images/normalize.png" : "pack://application:,,,/Resources/Images/maximize.png"));
		}

		private async void buttonClose_Click(object sender, RoutedEventArgs e)
		{
			await SaveScriptContent();
			Hide();
			Application.Current.Shutdown();
			System.Environment.Exit(0);
		}

		private async Task SaveScriptContent()
		{
			string content = await GetScriptContent();
			File.WriteAllText(Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "bin", "editor.lua"), content);
		}

		private void Window_MouseLeftButtonDown(object sender, MouseButtonEventArgs e) => DragMove();

		public void ExecuteScript(string scriptContent)
		{
			if (!_clientsWindow.ActiveClients.Any())
			{
				MessageBox.Show("No clients are currently selected. Please select at least one client before attempting to execute the script.", "No Client Selected", MessageBoxButton.OK, MessageBoxImage.Warning);
				return;
			}

			string status = _clientsWindow.GetCompilableStatus(scriptContent);
			if (status != "success") MessageBox.Show(status, "Compiler Error", MessageBoxButton.OK, MessageBoxImage.Exclamation);
			else _clientsWindow.ExecuteScript(scriptContent);
		}

		private async void buttonExecute_Click(object sender, RoutedEventArgs e)
		{
			string scriptContent = await GetScriptContent();
			ExecuteScript(scriptContent);
		}

		private async void buttonClear_Click(object sender, RoutedEventArgs e) => await script_editor.CoreWebView2.ExecuteScriptAsync("setText(\"\")");

		private async void buttonOpenFile_Click(object sender, RoutedEventArgs e)
		{
			var openFileDialog = new OpenFileDialog
			{
				Filter = "Script files (*.lua;*.luau;*.txt)|*.lua;*.luau;*.txt|All files (*.*)|*.*"
			};
			if (openFileDialog.ShowDialog() == true)
			{
				try
				{
					string content = await File.ReadAllTextAsync(openFileDialog.FileName);
					await SetScriptContent(content);
				}
				catch (Exception ex)
				{
					ShowError($"Error loading script: {ex.Message}");
				}
			}
		}

		private async void buttonSaveFile_Click(object sender, RoutedEventArgs e)
		{
			var saveFileDialog = new SaveFileDialog
			{
				Filter = "Script files (*.lua;*.luau;*.txt)|*.lua;*.luau;*.txt|All files (*.*)|*.*"
			};
			if (saveFileDialog.ShowDialog() == true)
			{
				try
				{
					string content = await GetScriptContent();
					await File.WriteAllTextAsync(saveFileDialog.FileName, content, Encoding.UTF8);
					MessageBox.Show("File saved successfully!", "Success", MessageBoxButton.OK, MessageBoxImage.Information);
				}
				catch (Exception ex)
				{
					ShowError($"Error saving file: {ex.Message}");
				}
			}
		}

		private void buttonShowMultinstance_Click(object sender, RoutedEventArgs e) => ToggleWindowVisibility(_clientsWindow);

		private void buttonShowScripts_Click(object sender, RoutedEventArgs e) => ToggleWindowVisibility(_scriptsWindow);

		private static void ToggleWindowVisibility(Window window)
		{
			if (window.IsVisible) window.Hide();
			else window.Show();
		}
	}
}
