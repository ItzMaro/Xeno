using System.IO;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Threading;

namespace XenoUI
{
	public partial class ScriptsWindow : Window
	{
		private readonly string scriptsDirectory;
		private readonly DispatcherTimer updateTimer;
		private readonly Dictionary<string, UIElement> scriptPanels;
		private readonly MainWindow _mainWindow;

		public ScriptsWindow(MainWindow mainWindow)
		{
			InitializeComponent();
			_mainWindow = mainWindow;
			scriptsDirectory = Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "scripts");
			scriptPanels = new Dictionary<string, UIElement>();

			MouseLeftButtonDown += (sender, e) => DragMove();

			Directory.CreateDirectory(scriptsDirectory);

			updateTimer = new DispatcherTimer
			{
				Interval = TimeSpan.FromSeconds(0.5)
			};
			updateTimer.Tick += (sender, e) => UpdateScripts();
			updateTimer.Start();

			LoadScripts();
		}

		private void LoadScripts()
		{
			foreach (var file in Directory.GetFiles(scriptsDirectory))
			{
				AddScriptPanel(file);
			}
		}

		private void UpdateScripts()
		{
			var currentFiles = new HashSet<string>(Directory.GetFiles(scriptsDirectory));

			foreach (var file in scriptPanels.Keys.Except(currentFiles).ToList())
			{
				RemoveScriptPanel(file);
			}

			foreach (var file in currentFiles.Except(scriptPanels.Keys))
			{
				AddScriptPanel(file);
			}
		}

		private void AddScriptPanel(string filePath)
		{
			var fileName = Path.GetFileName(filePath);

			var grid = new Grid
			{
				Margin = new Thickness(5),
				HorizontalAlignment = HorizontalAlignment.Stretch
			};

			grid.ColumnDefinitions.Add(new ColumnDefinition { Width = new GridLength(1, GridUnitType.Star) });
			grid.ColumnDefinitions.Add(new ColumnDefinition { Width = GridLength.Auto });

			var nameTextBlock = new TextBlock
			{
				Text = fileName,
				Foreground = System.Windows.Media.Brushes.White,
				VerticalAlignment = VerticalAlignment.Center,
				FontFamily = new System.Windows.Media.FontFamily("Cascadia Code"),
				FontSize = 14
			};
			Grid.SetColumn(nameTextBlock, 0);

			var executeButton = new Button
			{
				Content = "Execute",
				Margin = new Thickness(10, 0, 0, 0),
				Tag = filePath,
				Style = (Style)FindResource("DarkRoundedButtonStyle")
			};
			Grid.SetColumn(executeButton, 1);

			executeButton.Click += async (sender, e) =>
			{
				string scriptContent = await File.ReadAllTextAsync(filePath);
				_mainWindow.ExecuteScript(scriptContent);
			};

			var separatorBorder = new Border
			{
				BorderBrush = System.Windows.Media.Brushes.Gray,
				BorderThickness = new Thickness(0, 0, 0, 1),
				Margin = new Thickness(0, 5, 0, 0)
			};
			Grid.SetColumn(separatorBorder, 0);
			Grid.SetColumnSpan(separatorBorder, 2);

			grid.Children.Add(nameTextBlock);
			grid.Children.Add(executeButton);
			grid.Children.Add(separatorBorder);
			scripts_container.Children.Add(grid);
			scriptPanels[filePath] = grid;
		}

		private void RemoveScriptPanel(string filePath)
		{
			if (scriptPanels.TryGetValue(filePath, out var element))
			{
				scripts_container.Children.Remove(element);
				scriptPanels.Remove(filePath);
			}
		}

		private void buttonClose_Click(object sender, RoutedEventArgs e) => Hide();
	}
}
