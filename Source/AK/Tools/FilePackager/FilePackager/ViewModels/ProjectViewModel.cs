using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using FilePackager.Models;
using FilePackager.Commands;
using System.Windows.Input;
using System.Collections.ObjectModel;
using FilePackager.Base;
using System.Windows;
using FilePackager.Packaging;
using FilePackager.Undo;
using FilePackager.Base.WPF;
using System.Collections;
using System.IO;
using System.Xml.Linq;

namespace FilePackager.ViewModels
{
    public struct Tuple<T1, T2>
    {
        public readonly T1 Item1;
        public readonly T2 Item2;
        public Tuple(T1 item1, T2 item2) { Item1 = item1; Item2 = item2; }
    }

	public class ProjectViewModel : ViewModelBase
	{
		private DelegateCommand _browseInfoFileCommand;
        private DelegateCommand _browseOutputDirectory;
        private DelegateCommand _addContentToPackageCommand;
		private Project _project;
		
		private ObservableCollection<ContentItemViewModel> _contentItems = new ObservableCollection<ContentItemViewModel>();
		private bool _isDirty = false;
		
		private ManualPackagingInfoViewModel _manualPackagingInfoViewModel;

        // Map ID+Language to the content item
        private Dictionary<Tuple<UInt64,string>, ContentItemViewModel> _contentItemsPerId;

		private ReferenceManager _referenceManager = new ReferenceManager();

        private AK.Wwise.InfoFile.SoundBanksInfoBasePlatform _basePlatform;
        private string _soundBanksRoot;
        private string _sourceFilesRoot;
        private string _externalSourcesOutputRoot;
        
		public ProjectViewModel(Project project)
		{
			_project = project;
			RegisterModel(_project);

			_manualPackagingInfoViewModel = new ManualPackagingInfoViewModel(_project.ManualPackagingInfo, this);
			
			PropertyChanged += ProjectViewModel_PropertyChanged;

			ReferenceManager.ReferenceAdded += ReferenceManager_ReferenceChanged;
			ReferenceManager.ReferenceRemoved += ReferenceManager_ReferenceChanged;
		}

        public override void Detach()
        {
            base.Detach();

            _manualPackagingInfoViewModel.Detach();

            ReferenceManager.ReferenceAdded -= ReferenceManager_ReferenceChanged;
            ReferenceManager.ReferenceRemoved -= ReferenceManager_ReferenceChanged;
        }

        public override IEnumerable<string> Errors
        {
            get
            {
                return _manualPackagingInfoViewModel.Errors
                    .SafeConcat(base.Errors);
            }
        }

        public AK.Wwise.InfoFile.SoundBanksInfoBasePlatform BasePlatform
        {
            get { return _basePlatform; }
            set 
            {
                _basePlatform = value;
                OnPropertyChanged("BasePlatform");
            }
        }

		void ReferenceManager_ReferenceChanged(object sender, ReferenceManager.ReferenceArgs e)
		{
			e.ContentItem.ContentItem.OnReferencesChanged();
		}

		void ProjectViewModel_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
		{
			if (e.PropertyName == "InfoFilePath")
			{
				LoadContentItems();
			}
		}

		public void LoadContentItems()
		{
            using (new AutoCursor(Cursors.Wait))
            {
                string infoPath = InfoFilePathResolved;

                if (System.IO.File.Exists(infoPath))
                {
                    AK.Wwise.InfoFile.SoundBanksInfo soundBankInfo = null;
                    try
                    {
                        soundBankInfo = AK.Wwise.InfoFile.InfoFileHelpers.LoadInfoFile(infoPath);
                    }
                    catch (Exception e)
                    {
                        if (e.Message.Contains("xsd"))
                        {
                            MainViewModel.Instance.Application.ShowMessage(
                                string.Format(Properties.Resources.CouldNotFindXmlSchema, e.Message),
                                Severity.Error);
                        }
                        else
                        {
                            string message = string.Empty;
                            Exception current = e;
                            while (current != null)
                            {
                                message += current.Message + " ";
                                current = current.InnerException;
                            }
                            MainViewModel.Instance.Application.ShowMessage(
                                string.Format(Properties.Resources.CouldNotLoadInfoFile, message),
                                Severity.Error);
                        }
                    }
                    
                    if (soundBankInfo != null)
                    {
                        try
                        {
                            BasePlatform = soundBankInfo.BasePlatform;
                            _sourceFilesRoot = soundBankInfo.RootPaths.SourceFilesRoot;
                            _soundBanksRoot = soundBankInfo.RootPaths.SoundBanksRoot;
                            _externalSourcesOutputRoot = soundBankInfo.RootPaths.ExternalSourcesOutputRoot;

                            IEnumerable<AK.Wwise.InfoFile.FileDescriptorType> fileDescriptors =
                                soundBankInfo.SoundBanks.SoundBank.Cast<AK.Wwise.InfoFile.FileDescriptorType>()
                                .Concat( soundBankInfo.StreamedFiles.File.Cast<AK.Wwise.InfoFile.FileDescriptorType>()
                                .Union( soundBankInfo.MediaFilesNotInAnyBank.File.Cast<AK.Wwise.InfoFile.FileDescriptorType>()));

                            List<FilePackageGenerator.Context.ExternalSourceInfo> externalSources = new List<FilePackageGenerator.Context.ExternalSourceInfo>();

                            // Load External Sources
                            if (File.Exists(soundBankInfo.RootPaths.ExternalSourcesInputFile))
                            {
                                XElement root = XElement.Load(soundBankInfo.RootPaths.ExternalSourcesInputFile);
                                IEnumerable<XElement> sources = root.Elements("Source");

                                foreach (XElement source in sources)
                                {
                                    FilePackageGenerator.Context.ExternalSourceInfo info = new FilePackageGenerator.Context.ExternalSourceInfo();

                                    string sourcePath = source.Attribute("Path").Value;
                                    string destinationPath = string.Empty;
                                    XAttribute destination = source.Attribute("Destination");
                                    if (destination != null)
                                    {
                                        destinationPath = Path.Combine(_externalSourcesOutputRoot, destination.Value);
                                    }
                                    else
                                    {
                                        destinationPath = Path.Combine(_externalSourcesOutputRoot, sourcePath);
                                        destinationPath = FilePackager.Base.Extensions.PathCanonicalize(destinationPath);
                                    }

                                    destinationPath = Path.ChangeExtension(destinationPath, ".wem");
                                    
                                    info.Path = destinationPath;

                                    string name = FilePackager.Base.Extensions.PathRelativePathTo(_externalSourcesOutputRoot, true, destinationPath, false);
                                    info.Name = FilePackager.Base.Extensions.PathCanonicalize(name);

                                    externalSources.Add(info);
                                }
                            }

                            SetContentItems(fileDescriptors, externalSources);
                        }
                        catch (Exception e)
                        {
                            MainViewModel.Instance.Application.ShowMessage(
                                string.Format(Properties.Resources.GenericError, e.Message),
                                Severity.Error);
                        }
                    }
                }
                else
                {
                    if (!string.IsNullOrEmpty(infoPath))
                    {
                        MainViewModel.Instance.Application.ShowMessage(
                            string.Format(Properties.Resources.CouldNotFindInfoFile, infoPath),
                            Severity.Error);
                    }
                    
                    SetContentItems(null, null);
                }
            }

            // Clear the reference manager
            ManualPackagingInfo.Packages.ForEach(p => p.ResultingItems.RemoveAll());
            ReferenceManager.Clear();

            ManualPackagingInfo.InvalidateResultingItems();

            ManualPackagingInfo.Packages.SelectMany(p => p.Items).ForEach( i => i.OnIsMissingChanged() );
            ManualPackagingInfo.Packages.SelectMany(p => p.LayoutItems).ForEach(i => i.OnIsMissingChanged());

            ManualPackagingInfo.Packages.ForEach(p => p.OnMissingItemsChanged());
		}

        private void SetContentItems(
            IEnumerable<AK.Wwise.InfoFile.FileDescriptorType> fileDescriptors,
            IEnumerable<FilePackageGenerator.Context.ExternalSourceInfo> externalSources)
        {
            _contentItems.Clear();
            _contentItems.AddRange(fileDescriptors.SafeSelect(i => new ContentItemViewModel(i)));
            _contentItems.AddRange(externalSources.SafeSelect(i => new ContentItemViewModel(i)));

            // Maintain a map of ID to ContentItem
            try
            {
            _contentItemsPerId = _contentItems.ToDictionary(ci => new Tuple<UInt64, string>(ci.Id, ci.Language), ci => ci);
            }
            catch (ArgumentException e)
            {
                var duplicates = _contentItems.GroupBy(ci => new Tuple<UInt64, string>(ci.Id, ci.Language))
                    .Where(g => g.Count() > 1);

                if (duplicates.Any())
                {
                    var dup = duplicates.First().First();
                    throw new InvalidDataException("Could not load the content correctly.  Duplicate file identifiers (id) were found while loading SoundBanksInfo.xml.  First duplicate found is: " + dup.FileName + " [" + dup.FileType + "]");
                }
                else
                    throw e;
            }
            if (!UndoManager.Instance.IsBusy)
            {
                ManualPackagingInfo.UpdateLanguageList(fileDescriptors.SafeSelect(fd => fd.Language).Distinct());
            }
        }

		// Commands
		public ICommand BrowseInfoFileCommand
		{
			get { return CommandManagerHelper.CreateOnceCommand(ref _browseInfoFileCommand, BrowseInfoFile, null); }
		}

        public ICommand BrowseOutputDirectoryCommand
		{
            get { return CommandManagerHelper.CreateOnceCommand(ref _browseOutputDirectory, BrowseOutputDirectory, null); }
		}
        
        public ICommand AddContentToPackageCommand
		{
            get { return CommandManagerHelper.CreateOnceCommand(ref _addContentToPackageCommand, AddContentToPackage, CanAddContentToPackage); }
		}
        

		internal ReferenceManager ReferenceManager
		{
			get
			{
				return _referenceManager;
			}
		}

		internal Project Project
		{
			get
			{
				return _project;
			}
		}

		public static ProjectViewModel Current
		{
			get
			{
				return MainViewModel.Instance.Project;
			}
		}

		/// <summary>
		/// Gets the name of the Project.  This is the name of the file, without the extension.
		/// </summary>
		/// <value>The name.</value>
		public string Name
		{
			get
			{
				return System.IO.Path.GetFileNameWithoutExtension(_project.FilePath);
			}
		}

        public string FilePath
        {
            get { return _project.FilePath; }
        }

		public string InfoFilePath 
		{
			get { return _project.InfoFilePath; }
			set 
            {
                using (new UndoManager.EventGroupScope())
                {
                    SetValue(_project, "InfoFilePath", value);
                }
            }
		}

        /// <summary>
        /// Gets the info file path resolved.
        /// </summary>
        /// <value>The info file path resolved.</value>
        public string InfoFilePathResolved
        {
            get 
            {
                string infoPath = string.Empty;

                if (System.IO.Path.IsPathRooted(InfoFilePath))
                {
                    // If the path is complete, take it as-is.
                    infoPath = InfoFilePath;
                }
                else if (!string.IsNullOrEmpty(_project.FilePath))
                {
                    // Take the project path as the base directory
                    infoPath = Path.Combine(Path.GetDirectoryName(_project.FilePath), InfoFilePath);
                }

                return infoPath;
            }
        }

        public string OutputDirectory
        {
            get { return _project.OutputDirectory; }
            set { SetValue(_project, "OutputDirectory", value); }
        }

		public ObservableCollection<ContentItemViewModel> ContentItems
		{
			get
			{
				return _contentItems;
			}
		}

        public string SoundBanksRoot
        {
            get
            {
                // Override the path
                if (!string.IsNullOrEmpty(CommandLineViewModel.Instance.SoundBanksPath))
                    return CommandLineViewModel.Instance.SoundBanksPath;

                return _soundBanksRoot;
            }
        }

        public string SourceFilesRoot
        {
            get
            {
                // Override the path
                if (!string.IsNullOrEmpty(CommandLineViewModel.Instance.SourceFilesPath))
                    return CommandLineViewModel.Instance.SourceFilesPath;

                return _sourceFilesRoot;
            }
        }

		private void BrowseInfoFile()
		{ 
			Microsoft.Win32.OpenFileDialog dlg = new Microsoft.Win32.OpenFileDialog();
			dlg.Filter = "SoundBank Info File (.xml)|*.xml";

			// Show open file dialog box
			Nullable<bool> result = dlg.ShowDialog();

			if (result == true)
			{
                InfoFilePath = dlg.FileName;
			}
		}

        private void BrowseOutputDirectory()
        {
            System.Windows.Forms.FolderBrowserDialog dialog = new System.Windows.Forms.FolderBrowserDialog();
            dialog.Description = "Select Folder";
            dialog.ShowNewFolderButton = false;

            System.Windows.Forms.DialogResult result = dialog.ShowDialog();

            if (result == System.Windows.Forms.DialogResult.OK)//Intelisense does not give me the option for 'OK'  
            {
                OutputDirectory = dialog.SelectedPath;
            } 
        }

        private void AddContentToPackage(object parameter)
        {
            ProjectViewModel project = ProjectViewModel.Current;

            // Add selection to the current package
            IEnumerable<ContentItemViewModel> selectedItems = ((IList)parameter).Cast<ContentItemViewModel>();

            // Check for duplicate
            var newItems = from civm in selectedItems
                           where !project.ReferenceManager.HasExplicitReferencesToPackage(civm,_manualPackagingInfoViewModel.CurrentPackage)
                           select new PackageContentItem(civm.Id, civm.Language, civm.FileName);

            using (new UndoManager.EventGroupScope())
            {
                _manualPackagingInfoViewModel.CurrentPackage.Package.Items.AddRange(newItems);
            }
        }

        private bool CanAddContentToPackage(object parameter)
        {
            // Check that there is a selection in the content + there is a current package selected
            return ((IList)parameter).Count > 0 &&
                _manualPackagingInfoViewModel.CurrentPackage != null;
        }

		public ManualPackagingInfoViewModel ManualPackagingInfo
		{
			get
			{
				return _manualPackagingInfoViewModel;
			}
		}

		/// <summary>
		/// Gets or sets a value indicating whether the project is dirty (requires a save).
		/// </summary>
		/// <value><c>true</c> if this instance is dirty; otherwise, <c>false</c>.</value>
		internal bool IsDirty
		{
			get
			{
				return _isDirty;
			}
			set
			{
				_isDirty = value;
                OnPropertyChanged("IsDirty");
			}
		}

        /// <summary>
        /// Gets the content item.
        /// </summary>
        /// <param name="id">The id.</param>
        /// <param name="language">The language.</param>
        /// <returns>May return null if does not exist</returns>
		public ContentItemViewModel GetContentItem(UInt64 id, string language)
		{
            ContentItemViewModel item = null;

            // Only try the requested language
            if (_contentItemsPerId != null)
                _contentItemsPerId.TryGetValue(new Tuple<UInt64, string>(id, language), out item);

            return item;
		}

        /// <summary>
        /// Gets the content item.
        /// </summary>
        /// <param name="id">The id.</param>
        /// <param name="language">The language.</param>
        /// <returns>May return null if does not exist</returns>
        public ContentItemViewModel GetContentItemLanguageSafe(UInt64 id, string language)
        {
            ContentItemViewModel item = null;

            // First try the requested language
            if (_contentItemsPerId != null)
                _contentItemsPerId.TryGetValue(new Tuple<UInt64, string>(id, language), out item);

            // If nothing exist for the requested, we can try the SFX language, it might be a mixed bank
            if (_contentItemsPerId != null && item == null && language != "SFX")
                _contentItemsPerId.TryGetValue(new Tuple<UInt64, string>(id, "SFX"), out item);

            return item;
        }

        public void BuildAllPackages()
        {
            // Starts with all packages
            IEnumerable<PackageViewModel> packages = _manualPackagingInfoViewModel.Packages;

            // If a bank or language list is specified from the command line while using a File Packager Project,
            // use it to determine the package list
            if (CommandLineViewModel.Instance.IsGenerateMode &&
                !string.IsNullOrEmpty(CommandLineViewModel.Instance.ProjectPath) &&
                (CommandLineViewModel.Instance.Banks != null ||
                CommandLineViewModel.Instance.Languages != null))
            {
                IEnumerable<ContentItemViewModel> sourceItems = ContentItems;
                HashSet<string> banks = new HashSet<string>(CommandLineViewModel.Instance.Banks.SafeSelect(b => b.ToLowerInvariant()));
                HashSet<string> languages = new HashSet<string>(CommandLineViewModel.Instance.Languages.SafeSelect(b => b.ToLowerInvariant()));

                // Always generate the sfx language (as it is never sent in the language list)
                if (languages.Count > 0)
                    languages.Add("sfx");

                // Find the matching soundbanks with those names
                sourceItems = sourceItems.Where(ci =>
                    (banks.Count == 0 || banks.Contains(ci.FileName.ToLowerInvariant())) &&
                    (languages.Count == 0 || languages.Contains(ci.Language.ToLowerInvariant())));

                // Add the referenced streamed files
                sourceItems = sourceItems.Concat(sourceItems.SelectMany(si => si.ReferencedStreamedFiles));

                // Find the packages referencing those banks or streamed file, and remove duplicates
                packages = sourceItems.SelectMany(si => ProjectViewModel.Current.ReferenceManager.GetReferences(si).Select(pr => pr.Package)).Distinct();
            }

            BuildPackages(packages);
        }

        internal void BuildSelectedPackages()
        {
            BuildPackages(_manualPackagingInfoViewModel.Packages.Where(p => p.IsSelected));
        }
        
        internal bool CanBuildSelectedPackages()
        {
            return CanBuildPackages(_manualPackagingInfoViewModel.Packages.Where(p => p.IsSelected));
        }

        internal bool CanBuildAllPackages()
        {
            return CanBuildPackages(_manualPackagingInfoViewModel.Packages);
        }

        private bool CanBuildPackages(IEnumerable<PackageViewModel> packages)
        {
            return !string.IsNullOrEmpty(InfoFilePath) &&
                packages.Any();
        }

        private void BuildPackages(IEnumerable<PackageViewModel> packages)
        {
            if (Errors.Any())
                throw new InvalidDataException(Errors.ToString("\n"));

            // Setup the generation context
            FilePackageGenerator.Context context = new FilePackageGenerator.Context();

            string basePath = string.Empty;

            if (System.IO.Path.IsPathRooted(OutputDirectory))
                basePath = OutputDirectory;
            else
                basePath = Path.Combine(System.IO.Path.GetDirectoryName(InfoFilePathResolved), OutputDirectory);

            Directory.CreateDirectory(basePath);

            context.Global.BasePlatform = BasePlatform;
            context.Global.SoundBanksRoot = SoundBanksRoot;
            context.Global.SourceFilesRoot = SourceFilesRoot;

            context.Packages = packages.Select(p => new FilePackageGenerator.Context.PackageInfo()
            {
                FilePackageFilename = BuildPackageFileName(basePath, p.Name),
                BlockSize = _manualPackagingInfoViewModel.BlockSize,
                Files = p.ResolvedLayoutItems.Where(i=> !i.IsMissing ).Select( i => i.ContentItem ).Select(ci => new FilePackageGenerator.Context.PackageItem( ci.FileDescriptor, ci.ExternalSourceInfo ) ),
                MissingFiles = p.MissingItems.Select(i => i.FileName)
                    .Concat( p.ResolvedLayoutItems.Where(i=> i.IsMissing).Select(i => i.FileName)),
                MissingFromPackageFiles = p.LayoutItems.Where(i => i.IsMissingFromPackage).Select(i => i.FileName)
            });

            MainViewModel.Instance.Application.StartBuild(context);
        }

        private static string BuildPackageFileName(string basePath, string packageName)
        {
            if (System.IO.Path.IsPathRooted(packageName))
                return packageName;

            return Path.Combine( basePath, packageName );
        }
    }
}
