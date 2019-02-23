using System;
using System.Collections.Generic;
using System.Configuration;
using System.Data;
using System.Linq;
using System.Windows;
using FilePackager.Base;
using FilePackager.Views;
using FilePackager.Packaging;
using FilePackager.ViewModels;
using FilePackager.Models;
using FilePackager.Persist;

namespace FilePackager
{
	/// <summary>
	/// Interaction logic for App.xaml
	/// </summary>
    public partial class App : Application, IApplication
	{
        CommandLineViewModel _commandLine;

        private Dictionary<PackageViewModel, PackageLayoutView> _packageLayoutViews = new Dictionary<PackageViewModel, PackageLayoutView>();
        
        private static SplashScreen _splashScreen;

        /// <summary>
        /// Application Entry Point.
        /// </summary>
        [System.STAThreadAttribute()]
        [System.Diagnostics.DebuggerNonUserCodeAttribute()]
        public static void Main()
        {
            _splashScreen = new SplashScreen("Splash.png");
            _splashScreen.Show(false);
            FilePackager.App app = new FilePackager.App();
            app.InitializeComponent();
            app.Run();
        }

		private void OnStartup(object sender, StartupEventArgs e)
		{
            try
            {
                // Create the ViewModel
                ViewModels.MainViewModel mainViewModel = new ViewModels.MainViewModel(this);

                _commandLine = new CommandLineViewModel(e.Args);

                if (_commandLine.IsGenerateMode)
                {
                    if (!string.IsNullOrEmpty(_commandLine.ProjectPath))
                    {
                        mainViewModel.OpenProject(_commandLine.ProjectPath);
                    }
                    else
                    {
                        // Setup a temporary project from command line
                        Project project = new Project();
                        ProjectViewModel projectViewModel = new ProjectViewModel(project);
                        mainViewModel.Project = projectViewModel;

                        _commandLine.SetupProject(projectViewModel);
                    }

                    _splashScreen.Close(new TimeSpan(0));

                    // Generate packages
                    mainViewModel.Project.BuildAllPackages();

                    // The app won't terminate if no UI was shown
                    if (!_commandLine.ShowProgressUi)
                        Shutdown();
                }
                else
                {
                    if (!string.IsNullOrEmpty(_commandLine.ProjectPath))
                    {
                        mainViewModel.OpenProject(_commandLine.ProjectPath);
                    }
                    else
                    {
                        mainViewModel.NewProject();
                    }

                    mainViewModel.PropertyChanged += MainViewModel_PropertyChanged;

                    // Create the view, and set the ViewModel as the DataContext
                    Views.MainView mainView = new Views.MainView();
                    mainView.DataContext = mainViewModel;

                    _splashScreen.Close(new TimeSpan(0));

                    mainView.Show();
                }
            }
            catch (Exception exception)
            {
                _splashScreen.Close(new TimeSpan(0));

                ShowMessage(exception.Message, Severity.Error);
                Shutdown();
            }
        }

        public void ShowAboutView()
        {
            AboutView aboutView = new AboutView();
            aboutView.Owner = Application.Current.MainWindow;
            aboutView.ShowDialog();
        }

        public void StartBuild(FilePackageGenerator.Context context)
        {
            BuildViewModel buildViewModel = new BuildViewModel(context);
            buildViewModel.CloseMode = _commandLine.IsGenerateMode ? CloseMode.AlwaysClose : CloseMode.CloseOnlyWhenSuccess;

            if (_commandLine.IsGenerateMode)
                buildViewModel.Messages.CollectionChanged += Messages_CollectionChanged;

            RunMode mode = _commandLine.IsGenerateMode && !_commandLine.ShowProgressUi ? RunMode.Blocking : RunMode.Async;

            buildViewModel.Start(mode);

            if (mode == RunMode.Async)
            {
                BuildView buildView = new BuildView();
                buildView.DataContext = buildViewModel;
                buildView.ShowDialog();
            }

            buildViewModel.Detach();

            if (_commandLine.IsGenerateMode)
                buildViewModel.Messages.CollectionChanged -= Messages_CollectionChanged;
        }

        private void Messages_CollectionChanged(object sender, System.Collections.Specialized.NotifyCollectionChangedEventArgs e)
        {
            foreach (BuildViewModel.Message message in e.NewItems)
            {
                if (message.Severity != FilePackager.Packaging.Severity.Message)
                    System.Console.Error.WriteLine(message.ToString());
            }
        }

        public void EditPackageLayout(PackageViewModel package)
        {
            PackageLayoutView packageLayoutView;
            if (_packageLayoutViews.TryGetValue(package, out packageLayoutView))
            {
                packageLayoutView.Activate();
            }
            else
            {
                packageLayoutView = new PackageLayoutView();
                packageLayoutView.Owner = Application.Current.MainWindow;
                packageLayoutView.DataContext = package;

                _packageLayoutViews[package] = packageLayoutView;

                packageLayoutView.InputBindings.AddRange(Application.Current.MainWindow.InputBindings);
                packageLayoutView.Show();

                packageLayoutView.Closed += PackageLayoutView_Closed;
            }
        }

        void PackageLayoutView_Closed(object sender, EventArgs e)
        {
            PackageLayoutView packageLayoutView = (PackageLayoutView)sender;
            packageLayoutView.Closed -= PackageLayoutView_Closed;

            _packageLayoutViews.Remove(packageLayoutView.DataContext as PackageViewModel);
            packageLayoutView.DataContext = null;
        }

        void MainViewModel_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            if (e.PropertyName == "Project")
            {
                CloseAllPackageLayoutViews();
            }
        }

        public void CloseAllPackageLayoutViews()
        {
            foreach (PackageLayoutView view in _packageLayoutViews.Values.ToList())
            {
                view.Close();
            }
        }

        public void ShowMessage(string message, Severity severity)
        {
            if (_commandLine == null || _commandLine.IsGenerateMode)
            {
                if (severity == Severity.Error || severity == Severity.Warning)
                {
                    System.Console.Error.WriteLine(message);
                }
                else
                {
                    System.Console.Out.WriteLine(message);
                }
            }
            else
            {
                MessageBoxImage image = MessageBoxImage.None;
                switch (severity)
                {
                    case Severity.Message:
                        image = MessageBoxImage.Information;
                        break;
                    case Severity.Warning:
                        image = MessageBoxImage.Warning;
                        break;
                    case Severity.Error:
                        image = MessageBoxImage.Error;
                        break;
                }
                
                MessageBox.Show(
                    message,
                    AboutViewModel.ApplicationName,
                    MessageBoxButton.OK,
                    image);
            }
        }

        public void CheckMigrate()
        { 
            if (_commandLine == null || _commandLine.IsGenerateMode)
            {
                throw new NotSupportedException("File Packager project require migration. Migration is not possible in generate mode.  Please start the FilePackager in interactive mode to migrate the project.");
            }

            if (MessageBox.Show(
                FilePackager.Properties.Resources.CanMigrate,
                AboutViewModel.ApplicationName,
                MessageBoxButton.YesNo,
                MessageBoxImage.Question) == MessageBoxResult.No)
            {
                throw new OperationCanceledException("Migration canceled by the user.");
            }
        }

        public bool IsGenerateMode
        {
            get
            {
                return _commandLine != null && _commandLine.IsGenerateMode;
            }
        }
    }
}
