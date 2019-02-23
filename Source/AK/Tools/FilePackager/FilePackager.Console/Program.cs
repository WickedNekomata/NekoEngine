using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using FilePackager.ViewModels;
using FilePackager.Models;
using FilePackager.Packaging;

namespace FilePackager.Console
{
    class Application : IApplication
    {
        public FilePackager.Packaging.Severity MaxSeverityFound { get; set; }

        public Application()
        {
            MaxSeverityFound = FilePackager.Packaging.Severity.None;
        }

        /// <summary>
        /// Shows the about information.
        /// </summary>
        public void ShowAboutView()
        {
            System.Console.WriteLine( AboutViewModel.ApplicationName + " " + AboutViewModel.Build );
            System.Console.WriteLine( AboutViewModel.CopyrightNotice);
        }

        /// <summary>
        /// Starts the build in non-async mode.
        /// </summary>
        /// <param name="context">The context.</param>
        public void StartBuild(FilePackager.Packaging.FilePackageGenerator.Context context)
        {
            BuildViewModel buildViewModel = new BuildViewModel(context);
            buildViewModel.CloseMode = CloseMode.AlwaysClose;

            buildViewModel.PropertyChanged += BuildViewModel_PropertyChanged;
            buildViewModel.Messages.CollectionChanged += Messages_CollectionChanged;
            buildViewModel.Start(RunMode.Blocking);

            MaxSeverityFound = buildViewModel.MaxSeverityFound;

            buildViewModel.PropertyChanged -= BuildViewModel_PropertyChanged;
            buildViewModel.Messages.CollectionChanged -= Messages_CollectionChanged;
        }

        public void EditPackageLayout(PackageViewModel package)
        {
            throw new NotSupportedException();
        }

        private void BuildViewModel_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            BuildViewModel buildViewModel = (BuildViewModel)sender;
            if (e.PropertyName == "CurrentPackage")
            {
                System.Console.WriteLine( string.Format( FilePackager.Console.Properties.Resources.CurrentPackage, buildViewModel.CurrentPackage));
            }
        }

        private void Messages_CollectionChanged(object sender, System.Collections.Specialized.NotifyCollectionChangedEventArgs e)
        {
            foreach (BuildViewModel.Message message in e.NewItems)
            {
                if (message.Severity != FilePackager.Packaging.Severity.Message)
                    System.Console.Error.WriteLine(message.ToString());
                else
                    System.Console.WriteLine(message.ToString());
            }
        }

        public void ShowMessage(string message, Severity severity)
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

        public void CheckMigrate()
        {
            throw new NotSupportedException("File Packager project require migration.  Migration is not possible at command line.  Please use the FilePackage.App.exe to migrate the project.");
        }

        public bool IsGenerateMode
        {
            get { return true; }
        }
    }

    class Program
    {
        static int Main(string[] args)
        {
            Application application = new Application();

            // Show about info
            application.ShowAboutView();

            ViewModels.MainViewModel mainViewModel = new ViewModels.MainViewModel(application);

            try
            {
                // Read command line args
                CommandLineViewModel _commandLine = new CommandLineViewModel(args);

                if (!_commandLine.IsGenerateMode)
                    throw new InvalidOperationException("Use the \"-generate\" mode at command line.");

                if (!string.IsNullOrEmpty(_commandLine.ProjectPath))
                {
                    // Open the project
                    System.Console.WriteLine(string.Format(FilePackager.Console.Properties.Resources.OpeningProject, _commandLine.ProjectPath));

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

                // Generate packages
                mainViewModel.Project.BuildAllPackages();
            }
            catch (Exception exception)
            {
                System.Console.Error.WriteLine(exception.Message);
                return 1;
            }

            if (application.MaxSeverityFound == FilePackager.Packaging.Severity.Error)
                return 1;

            return 0;
        }
    }
}
