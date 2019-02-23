using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Input;
using System.IO;

using FilePackager.Commands;
using FilePackager.Models;
using FilePackager.Undo;
using FilePackager.Persist;
using FilePackager.Base;
using FilePackager.Packaging;
using FilePackager.Base.WPF;


namespace FilePackager.ViewModels
{
	public class MainViewModel : ViewModelBase
	{
		// Command
		// File
		private DelegateCommand _newProjectCommand;
		private DelegateCommand _openProjectCommand;
		private DelegateCommand _saveProjectCommand;
		private DelegateCommand _saveAsProjectCommand;
        private DelegateCommand _createLanguagePackagesCommand;
        private DelegateCommand _freezeSelectedPackagesCommand;

		// Edit
		private DelegateCommand _undoCommand;
		private DelegateCommand _redoCommand;

		// About/Exit
		private DelegateCommand _aboutCommand;
		private DelegateCommand _exitCommand;

        // Help
        private DelegateCommand _helpSearchCommand;
        private DelegateCommand _helpContentCommand;
        private DelegateCommand _helpIndexCommand;

        // Build
        private DelegateCommand _buildAllPackagesCommand;
        private DelegateCommand _buildSelectedPackagesCommand;
		
		private ProjectViewModel _projectViewModel;

        private IApplication _application;

        static private MainViewModel _instance = null;

		public MainViewModel(IApplication application)
		{
            _instance = this;

            _application = application;
		}

        static public MainViewModel Instance
        {
            get
            {
                return _instance;
            }
        }

        public IApplication Application
        {
            get
            {
                return _application;
            }
        }

        public override IEnumerable<string> Errors
        {
            get
            {
                return base.Errors.SafeConcat(_projectViewModel.Errors);
            }
        }

		/// <summary>
		/// Gets or sets the project.
		/// Can throw OperationCanceledException when user wants to cancels
		/// </summary>
		/// <value>The project.</value>
		public ProjectViewModel Project
		{
			get
			{
				return _projectViewModel;
			}
			set
			{
                if (_projectViewModel != null)
                {
                    _projectViewModel.Detach();
                    _projectViewModel.PropertyChanged -= Project_PropertyChanged;
                }

                _projectViewModel = value;

                if (_projectViewModel != null)
                {
                    _projectViewModel.PropertyChanged += Project_PropertyChanged;
                    _projectViewModel.LoadContentItems();
                }
				
                OnPropertyChanged("Project");
				OnPropertyChanged("Title");

                UndoManager.Instance.Clear();

                if (Project != null)
                    Project.IsDirty = false;
			}
		}

        void Project_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            if (e.PropertyName == "IsDirty")
            {
                OnPropertyChanged("Title");
            }
        }

		public bool CanCloseProject
		{
			get
			{
				bool canClose = true;

				// Check if the project is dirty
				Base.WPF.Helpers.PushTextBoxChanges();
				if (_projectViewModel != null && _projectViewModel.IsDirty)
				{
                    MessageBoxResult res = MessageBox.Show(Properties.Resources.DoYouWantToSave, System.Windows.Application.Current.MainWindow.Title, MessageBoxButton.YesNoCancel);

					if (res == MessageBoxResult.Yes)
					{
                        canClose = _saveProjectCommand.Execute(null);
					}
					else if (res == MessageBoxResult.Cancel)
					{
						canClose = false;
					}
				}

				return canClose;
			}
		}
		/// <summary>
		/// Gets the title.
		/// </summary>
		/// <value>The title from the application name and current project name.</value>
		public string Title
		{
			get
			{
                string appName = string.Format(Properties.Resources.ApplicationName, AK.Wwise.Version.VersionName);
                if (Project == null || string.IsNullOrEmpty(Project.Name))
				{
					return appName;
				}
                return string.Format(Properties.Resources.MainWindowTitle, appName, Project.Name, Project.IsDirty ? "*" : "");
			}
		}
		
		// Commands
		public ICommand NewProjectCommand
		{
			get { return CommandManagerHelper.CreateOnceCommand(ref _newProjectCommand, NewProject, null); }
		}

		public ICommand OpenProjectCommand
		{
			get { return CommandManagerHelper.CreateOnceCommand(ref _openProjectCommand, OpenProject, null); }
		}

		public ICommand SaveProjectCommand
		{
            get { return CommandManagerHelper.CreateOnceCommand(ref _saveProjectCommand, SaveProject, null, Properties.Resources.SaveProject); }
		}

		public ICommand SaveAsProjectCommand
		{
            get { return CommandManagerHelper.CreateOnceCommand(ref _saveAsProjectCommand, SaveAsProject, null, Properties.Resources.SaveAsProject); }
		}

        public ICommand CreateLanguagePackagesCommand
        {
            get { return CommandManagerHelper.CreateOnceCommand(ref _createLanguagePackagesCommand, CreateLanguagePackages, null, Properties.Resources.CreateLanguagePackages); }
        }

        public ICommand FreezeSelectedPackagesCommand
        {
            get { return CommandManagerHelper.CreateOnceCommand(ref _freezeSelectedPackagesCommand, FreezeSelectedPackages, null, Properties.Resources.FreezeSelectedPackages); }
        }

        public ICommand UndoCommand
		{
			get { return CommandManagerHelper.CreateOnceCommand(ref _undoCommand, Undo, CanUndo); }
		}

		public ICommand RedoCommand
		{
			get { return CommandManagerHelper.CreateOnceCommand(ref _redoCommand, Redo, CanRedo); }
		}

		public ICommand ExitCommand
		{
			get { return CommandManagerHelper.CreateOnceCommand(ref _exitCommand, Exit, null); }
		}

		public ICommand HelpSearchCommand
		{
			get { return CommandManagerHelper.CreateOnceCommand(ref _helpSearchCommand, HelpSearch, null); }
		}

		public ICommand HelpContentCommand
		{
			get { return CommandManagerHelper.CreateOnceCommand(ref _helpContentCommand, HelpContent, null); }
		}

		public ICommand HelpIndexCommand
		{
			get { return CommandManagerHelper.CreateOnceCommand(ref _helpIndexCommand, HelpIndex, null); }
		}

		public ICommand AboutCommand
		{
			get { return CommandManagerHelper.CreateOnceCommand(ref _aboutCommand, About, null); }
		}

        public ICommand BuildAllPackagesCommand
		{
            get { return CommandManagerHelper.CreateOnceCommand(ref _buildAllPackagesCommand, BuildAllPackages, CanBuildAllPackages, Properties.Resources.BuildAllPackages); }
		}

        public ICommand BuildSelectedPackagesCommand
        {
            get { return CommandManagerHelper.CreateOnceCommand(ref _buildSelectedPackagesCommand, BuildSelectedPackages, CanBuildSelectedPackages, Properties.Resources.BuildSelectedPackages); }
        }

		public void NewProject()
		{
			if (CanCloseProject)
			{
				Project project = new Project();
                Package defaultPackage = new Package() { Name = Properties.Resources.DefaultPackageName };
				project.ManualPackagingInfo.Packages.Add(defaultPackage);
				project.ManualPackagingInfo.UnassignedBanksPackageId = defaultPackage.Id;
				project.ManualPackagingInfo.UnassignedStreamsPackageId = defaultPackage.Id;
                project.ManualPackagingInfo.UnassignedExternalsPackageId = defaultPackage.Id;
                project.ManualPackagingInfo.UnassignedLooseMediaPackageId = defaultPackage.Id;
				Project = new ProjectViewModel(project);
			}
		}

		private void OpenProject()
		{
			if (CanCloseProject)
			{
				Microsoft.Win32.OpenFileDialog dlg = new Microsoft.Win32.OpenFileDialog();
				dlg.Filter = "Wwise File Packager Project (.wfpproj)|*.wfpproj";

				// Show open file dialog box
				Nullable<bool> result = dlg.ShowDialog();

				if (result == true)
				{
                    OpenProject(dlg.FileName);
				}
			}
		}

        public void OpenProject(string projectFilePath)
        {
            try
            {
                // Open document
                Project project = ProjectPersist.Load(projectFilePath);
                Project = new ProjectViewModel(project);
            }
            catch (Exception e)
            {
                MainViewModel.Instance.Application.ShowMessage(
                    string.Format(Properties.Resources.ErrorLoadingProject, e.Message),
                    Severity.Error);
            }
        }

		private void SaveProject()
		{
            Base.WPF.Helpers.PushTextBoxChanges();

            if (Errors.Any())
                throw new InvalidDataException(Errors.ToString("\n"));

			if (string.IsNullOrEmpty(_projectViewModel.Project.FilePath))
			{
				SaveAsProject();
			}
			else
			{
                try
                {
                    ProjectPersist.Save(_projectViewModel.Project);
                    _projectViewModel.IsDirty = false;
                }
                catch (Exception e)
                {
                    MainViewModel.Instance.Application.ShowMessage(
                        string.Format(Properties.Resources.ErrorSavingProject, e.Message),
                        Severity.Error);
                }
            }
		}

		private void SaveAsProject()
		{
			// Configure save file dialog box
			Microsoft.Win32.SaveFileDialog dlg = new Microsoft.Win32.SaveFileDialog();
			dlg.FileName = "Project";
			dlg.DefaultExt = ".wfpproj";
			dlg.Filter = "Text documents (.wfpproj)|*.wfpproj";

			// Show save file dialog box
			Nullable<bool> result = dlg.ShowDialog();

			// Process save file dialog box results
			if (result == true)
			{
				_projectViewModel.Project.FilePath = dlg.FileName;
				SaveProject();
			}
		}

        private void CreateLanguagePackages()
        {
            Project.ManualPackagingInfo.CreateLanguagePackages();
        }

        private void FreezeSelectedPackages()
        {
            Project.ManualPackagingInfo.FreezeSelectedPackages();
        }
		private bool CanUndo()
		{
			return UndoManager.Instance.CanUndo();
		}

		private void Undo()
		{
			UndoManager.Instance.Undo();
		}

		private bool CanRedo()
		{
			return UndoManager.Instance.CanRedo();
		}

		private void Redo()
		{
			UndoManager.Instance.Redo();
		}

		private void Exit()
		{
            System.Windows.Application.Current.Shutdown();
		}

		private void About()
		{
            _application.ShowAboutView();
		}

        private void BuildAllPackages()
        {
            Base.WPF.Helpers.PushTextBoxChanges();
            _projectViewModel.BuildAllPackages();
        }

        private bool CanBuildAllPackages()
        {
            return _projectViewModel.CanBuildAllPackages();
        }

        private void BuildSelectedPackages()
        {
            Base.WPF.Helpers.PushTextBoxChanges();
            _projectViewModel.BuildSelectedPackages();
        }

        private bool CanBuildSelectedPackages()
        { 
            return _projectViewModel.CanBuildSelectedPackages();
        }

        private void HelpSearch()
        {
            ChmHelp.Show(System.Windows.Forms.HelpNavigator.Find);
        }

        private void HelpContent()
        {
            ChmHelp.Show(FilePackager.TopicAlias.FilePackagerManagingFilePackages);
        }

        private void HelpIndex()
        {
            ChmHelp.Show(System.Windows.Forms.HelpNavigator.Index);
        }
    }
}
