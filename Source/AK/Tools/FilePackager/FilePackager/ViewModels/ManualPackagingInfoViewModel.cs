using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using FilePackager.Models;
using System.Collections.ObjectModel;
using FilePackager.Commands;
using System.Windows.Input;
using FilePackager.Undo;
using System.ComponentModel;
using System.Windows.Data;
using FilePackager.Base;
using FilePackager.Base.WPF;
using System.Collections;
using System.Windows;
using System.Windows.Threading;
using System.Threading;
using System.IO;

namespace FilePackager.ViewModels
{
	public class ManualPackagingInfoViewModel : ViewModelBase
	{
		private ManualPackagingInfo _manualPackagingInfo;

		private DelegateCommand _addPackageCommand;
		private DelegateCommand _removePackageCommand;

		private UndoableObservableCollection<PackageViewModel, Package> _packages;
		private Dictionary<Guid, PackageViewModel> _packagesPerId = new Dictionary<Guid,PackageViewModel>();

        private UndoableObservableCollection<LanguagePackageIdsViewModel, LanguagePackageIds> _defaultLanguagePackageIds;

        private PackageViewModel _nonePackage;
		private ICollectionView _packagesView;

        private BackgroundWorker _resultingItemsWorker = new BackgroundWorker();

		private PackageViewModel CreatePackageViewModel(Package package)
		{
			return new PackageViewModel(package,this);
		}

        private static LanguagePackageIdsViewModel CreateLanguagePackageIdsViewModel(LanguagePackageIds languagePackageIds)
        {
            return new LanguagePackageIdsViewModel(languagePackageIds);
        }

		public ManualPackagingInfoViewModel(ManualPackagingInfo manualPackagingInfo, ProjectViewModel projectViewModel)
		{
			_manualPackagingInfo = manualPackagingInfo;
			RegisterModel(_manualPackagingInfo);

            _nonePackage = CreatePackageViewModel(new Package() { Name = "None" });

			_packages = new UndoableObservableCollection<PackageViewModel, Package>(_manualPackagingInfo.Packages, CreatePackageViewModel);
			_packages.CollectionChanged += _packages_CollectionChanged;

            _defaultLanguagePackageIds = new UndoableObservableCollection<LanguagePackageIdsViewModel, LanguagePackageIds>(_manualPackagingInfo.DefaultLanguagePackageIds, CreateLanguagePackageIdsViewModel);

			// Init Package per ID
			foreach (PackageViewModel item in _packages)
			{
				RegisterPackage(item);
			}

            _resultingItemsWorker.DoWork += ResultingItemsWorker_DoWork;
            _resultingItemsWorker.WorkerSupportsCancellation = true;

            projectViewModel.PropertyChanged += ProjectViewModel_PropertyChanged;
		}

        void ProjectViewModel_PropertyChanged(object sender, PropertyChangedEventArgs e)
        {
        }

        public override void Detach()
        {
            base.Detach();

            foreach (PackageViewModel item in _packages)
            {
                UnregisterPackage(item);
            }

            _defaultLanguagePackageIds.Detach();

            _packages.CollectionChanged -= _packages_CollectionChanged;
            _packages.Detach();

            _nonePackage.Detach();

            if (_packagesView != null)
            {
                _packagesView.CurrentChanged -= PackagesView_CurrentChanged;
            }

            _resultingItemsWorker.DoWork -= ResultingItemsWorker_DoWork;

            ProjectViewModel.Current.PropertyChanged -= ProjectViewModel_PropertyChanged;
        }

        public override IEnumerable<string> Errors
        {
            get
            {
                return _defaultLanguagePackageIds.SelectMany(i => i.Errors)
                    .SafeConcat(_packages.SelectMany(p => p.Errors))
                    .SafeConcat(base.Errors);
            }
        }
        
        protected override void OnModelPropertyChanged(object sender, PropertyChangedEventArgs e)
        {
            base.OnModelPropertyChanged(sender, e);

            if (e.PropertyName == "UnassignedBanksPackageId")
			{
				OnPropertyChanged("UnassignedBanksPackage");
				InvalidateResultingItems();
			}
			else if (e.PropertyName == "UnassignedStreamsPackageId")
			{
				OnPropertyChanged("UnassignedStreamsPackage");
				InvalidateResultingItems();
			}
            else if (e.PropertyName == "UnassignedExternalsPackageId")
			{
                OnPropertyChanged("UnassignedExternalsPackage");
				InvalidateResultingItems();
			}
            else if (e.PropertyName == "UnassignedLooseMediaPackageId")
            {
                OnPropertyChanged("UnassignedLooseMediaPackage");
                InvalidateResultingItems();
            }
		}

        public void InvalidateResultingItems()
        {
            if (UndoManager.Instance.IsBusy || UndoManager.Instance.HasActiveGroup)
            {
                // Wait until the last group is closed
                UndoManager.Instance.RegisterOnceToIsBusyOrGroupFinished(UndoManager_IsBusyOrGroupFinished);
            }
            else
            {
                RecalcResultingItems();
            }
        }

        void UndoManager_IsBusyOrGroupFinished(object sender, EventArgs e)
        {
            UndoManager.Instance.IsBusyOrGroupFinished -= UndoManager_IsBusyOrGroupFinished;
            RecalcResultingItems();
        }


        internal void RecalcResultingItems()
        {
            if (MainViewModel.Instance.Application.IsGenerateMode)
            {
                // Do it synchronously
                ResultingItemsWorker_DoWork(_resultingItemsWorker, new DoWorkEventArgs(null) { Cancel = false });
            }
            else
            {
                // If another instance is waiting for the cancellation, don't wait
                if (_resultingItemsWorker.CancellationPending)
                    return;

                if (_resultingItemsWorker.IsBusy)
                {
                    _resultingItemsWorker.CancelAsync();

                    // Wait until cancellation has occured
                    while (_resultingItemsWorker.IsBusy)
                    {
                        System.Threading.Thread.Sleep(40);
                        System.Windows.Forms.Application.DoEvents();
                    }
                }

                _resultingItemsWorker.RunWorkerAsync();
            }
        }

        class PackageResultingItems : ObservableCollection<ContentItemViewModelReference>
        {
            private PackageViewModel _package;
            private ReferenceManager _referenceManager;

            public PackageResultingItems(PackageViewModel package, ReferenceManager referenceManager)
            {
                _package = package;
                _referenceManager = referenceManager;

                CollectionChanged += OnCollectionChanged;
            }

            private void OnCollectionChanged(object sender, System.Collections.Specialized.NotifyCollectionChangedEventArgs e)
            {
                switch (e.Action)
                {
                    case System.Collections.Specialized.NotifyCollectionChangedAction.Add:
                        // Add references
                        foreach (ContentItemViewModelReference item in e.NewItems)
                        {
                            _referenceManager.AddReference(item, _package);
                        }
                        break;

                    case System.Collections.Specialized.NotifyCollectionChangedAction.Remove:
                        // Remove references
                        foreach (ContentItemViewModelReference item in e.OldItems)
                        {
                            _referenceManager.RemoveReference(item, _package);
                        }
                        break;
                }
            }
        }

        class ContentItemViewModelReferenceComparer : IEqualityComparer<ContentItemViewModelReference>
        {
            public bool Equals(ContentItemViewModelReference x, ContentItemViewModelReference y)
            {
                return x.ContentItem.Id == y.ContentItem.Id &&
                    x.ContentItem.Language == y.ContentItem.Language;
            }

            public int GetHashCode(ContentItemViewModelReference obj)
            {
                return (obj.ContentItem.Language + obj.ContentItem.Id.ToString()).GetHashCode();
            }
        }

        void ResultingItemsWorker_DoWork(object sender, DoWorkEventArgs e)
        {
            if (_resultingItemsWorker.CancellationPending)
                return;

            ReferenceManager referenceManager = new ReferenceManager();
            Dictionary<PackageViewModel, PackageResultingItems> newItems =
                _packages.ToDictionary(p => p, p => new PackageResultingItems(p, referenceManager));

            ProjectViewModel project = ProjectViewModel.Current;

            // First clear all packages and copy the included items and their associated streams
            foreach (PackageViewModel package in Packages)
            {
                // Copy the manually added items
                newItems[package].AddRange(
                    package.Items
                    .Where(i => i.ContentItem != null)  // Skip missing references
                    .Where(i => i.CanIncludeExplicitly)
                    .Select(i => new ContentItemViewModelReference() { ContentItem = i.ContentItem, IsExplicit = true }));

                // Copy implicit items
                newItems[package].AddRange(
                    package.Items
                    .Where(i => i.ContentItem != null)
                    .SelectMany( i => i.ImplicitItems )
                    .Where( i => i != null && !referenceManager.HasExplicitReferencesToPackage(i, package) )
                    .Select(i => new ContentItemViewModelReference() { ContentItem = i, IsExplicit = false }));

                if (_resultingItemsWorker.CancellationPending)
                    return;
            }

            // Add unassigned bank items per language
            foreach (LanguagePackageIdsViewModel lpivm in DefaultLanguagePackageIds)
            {
                if (lpivm.BankPackage != _nonePackage)
                {
                    var noRef = project.ContentItems.Where(ci =>
                        ci.FileType == FileType.SoundBank &&
                        ci.Language == lpivm.Language &&
                        referenceManager.GetReferences(ci) == null);

                    newItems[lpivm.BankPackage].AddRange(noRef.Select(i => new ContentItemViewModelReference() { ContentItem = i, IsExplicit = false }));
                }
                if (lpivm.StreamPackage != _nonePackage)
                {
                    var noRef = project.ContentItems.Where(ci =>
                        ci.FileType == FileType.StreamedFile &&
                        ci.Language == lpivm.Language &&
                        referenceManager.GetReferences(ci) == null);

                    newItems[lpivm.StreamPackage].AddRange(noRef.Select(i => new ContentItemViewModelReference() { ContentItem = i, IsExplicit = false }));
                }
                if (lpivm.LooseMediaPackage != _nonePackage)
                {
                    var noRef = project.ContentItems.Where(ci =>
                        ci.FileType == FileType.LooseMedia &&
                        ci.Language == lpivm.Language &&
                        referenceManager.GetReferences(ci) == null);

                    newItems[lpivm.LooseMediaPackage].AddRange(noRef.Select(i => new ContentItemViewModelReference() { ContentItem = i, IsExplicit = false }));
                }

                if (_resultingItemsWorker.CancellationPending)
                    return;
            }
            
            // Add unassigned bank items
            if (UnassignedBanksPackage != _nonePackage)
            {
                var noRef = project.ContentItems.Where(ci => ci.FileType == FileType.SoundBank && referenceManager.GetReferences(ci) == null);

                newItems[UnassignedBanksPackage].AddRange(noRef.Select(i => new ContentItemViewModelReference() { ContentItem = i, IsExplicit = false }));
            }

            if (_resultingItemsWorker.CancellationPending)
                return;
            
            // Add unassigned stream items
            if (UnassignedStreamsPackage != _nonePackage)
            {
                var noRef = project.ContentItems.Where(ci => ci.FileType == FileType.StreamedFile && referenceManager.GetReferences(ci) == null);

                newItems[UnassignedStreamsPackage].AddRange(noRef.Select(i => new ContentItemViewModelReference() { ContentItem = i, IsExplicit = false }));
            }

            if (_resultingItemsWorker.CancellationPending)
                return;

            // Add unassigned externals items
            if (UnassignedExternalsPackage != _nonePackage)
            {
                var noRef = project.ContentItems.Where(ci => ci.FileType == FileType.ExternalSource && referenceManager.GetReferences(ci) == null);

                newItems[UnassignedExternalsPackage].AddRange(noRef.Select(i => new ContentItemViewModelReference() { ContentItem = i, IsExplicit = false }));
            }

            // Add unassigned loose media items
            if (UnassignedLooseMediaPackage != _nonePackage)
            {
                var noRef = project.ContentItems.Where(ci => ci.FileType == FileType.LooseMedia && referenceManager.GetReferences(ci) == null);

                newItems[UnassignedLooseMediaPackage].AddRange(noRef.Select(i => new ContentItemViewModelReference() { ContentItem = i, IsExplicit = false }));
            }

            if (_resultingItemsWorker.CancellationPending)
                return;

            // Finally add/remove the items
            if (MainViewModel.Instance.Application.IsGenerateMode)
            {
                PushResultingItems(newItems);
            }
            else
            {
                // Switch thread when running in background
                Application.Current.Dispatcher.BeginInvoke(DispatcherPriority.Normal,
                    (Action)delegate
                    {
                        using (new AutoCursor(Cursors.Wait))
                        {
                            PushResultingItems(newItems);
                        }
                    });
            }
        }

        private void PushResultingItems(Dictionary<PackageViewModel, PackageResultingItems> newItems)
        {
            ContentItemViewModelReferenceComparer comparer = new ContentItemViewModelReferenceComparer();

            foreach (PackageViewModel package in Packages)
            {
                IEnumerable<ContentItemViewModelReference> toAdd = newItems[package].Except(package.ResultingItems, comparer);
                IEnumerable<ContentItemViewModelReference> toRemove = package.ResultingItems.Except(newItems[package], comparer);

                package.ResultingItems.RemoveRange(toRemove);
                package.ResultingItems.AddRange(toAdd);

                // Notify our package that this is finished
                package.OnResultingItemsChanged();
                package.OnUnlayoutItemsChanged();
            }
        }

		void _packages_CollectionChanged(object sender, System.Collections.Specialized.NotifyCollectionChangedEventArgs e)
		{
			switch (e.Action)
			{
				case System.Collections.Specialized.NotifyCollectionChangedAction.Add:
					foreach (PackageViewModel item in e.NewItems)
					{
						RegisterPackage(item);
					}
					break;

				case System.Collections.Specialized.NotifyCollectionChangedAction.Remove:
					foreach (PackageViewModel item in e.OldItems)
					{
						UnregisterPackage(item);

                        if (!UndoManager.Instance.IsBusy)
                        {
                            // If any of the package in the unassigned rules get deleted, set the package to none
                            RemovePackageFromAssignations(item.Package.Id);
                        }
					}
					break;

				case System.Collections.Specialized.NotifyCollectionChangedAction.Move:
				case System.Collections.Specialized.NotifyCollectionChangedAction.Replace:
				case System.Collections.Specialized.NotifyCollectionChangedAction.Reset:
				default:
					throw new NotSupportedException();
			}

			OnPropertyChanged("PackagesWithNone");

            InvalidateResultingItems();
		}

        private void RemovePackageFromAssignations(Guid packageId)
        {
            if (packageId == UnassignedBanksPackageId)
            {
                UnassignedBanksPackage = _nonePackage;
            }
            if (packageId == UnassignedStreamsPackageId)
            {
                UnassignedStreamsPackage = _nonePackage;
            }
            if (packageId == UnassignedExternalsPackageId)
            {
                UnassignedExternalsPackage = _nonePackage;
            }
            if (packageId == UnassignedLooseMediaPackageId)
            {
                UnassignedLooseMediaPackage = _nonePackage;
            }

            _defaultLanguagePackageIds
                .Where(i => i.BankPackageId == packageId)
                .ForEach(i => i.BankPackage = _nonePackage);
            _defaultLanguagePackageIds
                .Where(i => i.StreamPackageId == packageId)
                .ForEach(i => i.StreamPackage = _nonePackage);
            _defaultLanguagePackageIds
                .Where(i => i.LooseMediaPackageId == packageId)
                .ForEach(i => i.LooseMediaPackage = _nonePackage);
        }

		private void UnregisterPackage(PackageViewModel item)
		{
            item.PropertyChanged -= Package_PropertyChanged;
			item.Items.CollectionChanged -= PackageItems_CollectionChanged;
			if (!_packagesPerId.Remove(item.Id))
				throw new InvalidOperationException("Could not find package.");
		}

		private void RegisterPackage(PackageViewModel item)
		{
			_packagesPerId.Add(item.Id, item);
			item.Items.CollectionChanged += PackageItems_CollectionChanged;
            item.PropertyChanged += Package_PropertyChanged;
		}

        void Package_PropertyChanged(object sender, PropertyChangedEventArgs e)
        {
            if (e.PropertyName == "Name")
            {
                OnPropertyChanged("PackagesWithNone");
            }
        }

		void PackageItems_CollectionChanged(object sender, System.Collections.Specialized.NotifyCollectionChangedEventArgs e)
		{
			InvalidateResultingItems();
		}

		public ObservableCollection<PackageViewModel> Packages
		{
			get
			{
				return _packages;
			}
		}

        public ObservableCollection<LanguagePackageIdsViewModel> DefaultLanguagePackageIds
        {
            get
            {
                return _defaultLanguagePackageIds;
            }
        }

		public IEnumerable<PackageViewModel> PackagesWithNone
		{
			get
			{
				List<PackageViewModel> packages = new List<PackageViewModel>();
                packages.AddRange(_packages);
                packages.Sort(delegate(PackageViewModel a, PackageViewModel b)
                {
                    return a.Name.NaturalCompareTo(b.Name);
                });
                packages.Insert(0,_nonePackage);
                return packages;
			}
		}

		public uint BlockSize
		{
			get { return _manualPackagingInfo.BlockSize; }
			set { SetValue(_manualPackagingInfo, "BlockSize", value); }
		}

		private Guid UnassignedBanksPackageId
		{
			get { return _manualPackagingInfo.UnassignedBanksPackageId; }
			set { SetValue(_manualPackagingInfo, "UnassignedBanksPackageId", value); }
		}

		public PackageViewModel UnassignedBanksPackage
		{
			get { return UnassignedBanksPackageId == Guid.Empty ? _nonePackage : _packagesPerId[UnassignedBanksPackageId]; }
			set { UnassignedBanksPackageId = (value == _nonePackage) || (value == null) ? Guid.Empty : value.Id; }
		}

		private Guid UnassignedStreamsPackageId
		{
			get { return _manualPackagingInfo.UnassignedStreamsPackageId; }
			set { SetValue(_manualPackagingInfo, "UnassignedStreamsPackageId", value); }
		}

		public PackageViewModel UnassignedStreamsPackage
		{
			get { return UnassignedStreamsPackageId == Guid.Empty ? _nonePackage : _packagesPerId[UnassignedStreamsPackageId]; }
			set { UnassignedStreamsPackageId = (value == _nonePackage) || (value == null) ? Guid.Empty : value.Id; }
		}

        private Guid UnassignedExternalsPackageId
        {
            get { return _manualPackagingInfo.UnassignedExternalsPackageId; }
            set { SetValue(_manualPackagingInfo, "UnassignedExternalsPackageId", value); }
        }

        public PackageViewModel UnassignedExternalsPackage
        {
            get { return UnassignedExternalsPackageId == Guid.Empty ? _nonePackage : _packagesPerId[UnassignedExternalsPackageId]; }
            set { UnassignedExternalsPackageId = (value == _nonePackage) || (value == null) ? Guid.Empty : value.Id; }
        }

        private Guid UnassignedLooseMediaPackageId
        {
            get { return _manualPackagingInfo.UnassignedLooseMediaPackageId; }
            set { SetValue(_manualPackagingInfo, "UnassignedLooseMediaPackageId", value); }
        }

        public PackageViewModel UnassignedLooseMediaPackage
        {
            get { return UnassignedLooseMediaPackageId == Guid.Empty ? _nonePackage : _packagesPerId[UnassignedLooseMediaPackageId]; }
            set { UnassignedLooseMediaPackageId = (value == _nonePackage) || (value == null) ? Guid.Empty : value.Id; }
        }

        public PackageViewModel NonePackage
        {
            get
            {
                return _nonePackage;
            }
        }

		public ICommand AddPackageCommand
		{
			get { return CommandManagerHelper.CreateOnceCommand(ref _addPackageCommand, AddPackage, null); }
		}

		public ICommand RemovePackageCommand
		{
			get { return CommandManagerHelper.CreateOnceCommand(ref _removePackageCommand, RemovePackage, CanRemovePackage); }
		}

		private void AddPackage()
		{
			// Find a unique name
            string name = NameHelpers.FindUniqueName(
                Packages.Select(p => p.Name),
                Properties.Resources.NewPackageName,
                StringComparison.OrdinalIgnoreCase);

            Package package = new Package() { Name = name };
			_manualPackagingInfo.Packages.Add(package);

            // Trigger the rename, but let the UI item to be loaded and data-bound
            Application.Current.Dispatcher.BeginInvoke(System.Windows.Threading.DispatcherPriority.Loaded,
                (Action)delegate
                {
                    _packagesPerId[package.Id].Rename();
                });
		}

		private void RemovePackage(object parameter)
		{
            using (new UndoManager.EventGroupScope())
            {
                // Remove selected packages
                foreach (PackageViewModel package in ((IList)parameter).Cast<PackageViewModel>().ToList())
                {
                    package.Delete();
                    _manualPackagingInfo.Packages.Remove(package.Package);
                }
            }
		}

        private bool CanRemovePackage(object parameter)
		{
            return ((IList)parameter).Count > 0;
		}

		public ICollectionView PackagesView
		{
			get
			{
				if (_packagesView == null)
				{ 
					_packagesView = CollectionViewSource.GetDefaultView(Packages);
					_packagesView.CurrentChanged += PackagesView_CurrentChanged;
				}
				return _packagesView;
			}
		}

		void PackagesView_CurrentChanged(object sender, EventArgs e)
		{
			OnPropertyChanged("CurrentPackage");
		}

		public PackageViewModel CurrentPackage
		{
			get { return PackagesView.CurrentItem as PackageViewModel; }
			set { PackagesView.MoveCurrentTo(value); }
		}

        public PackageViewModel GetPackagePerId(Guid packageId)
        {
            return _packagesPerId[packageId];
        }

        public void UpdateLanguageList(IEnumerable<string> newLanguages)
        {
            IEnumerable<string> oldLanguages = _manualPackagingInfo.DefaultLanguagePackageIds.Select(i => i.Language);

            IEnumerable<string> toRemove = oldLanguages.Except(newLanguages).ToList();
            IEnumerable<string> toAdd = newLanguages.Except(oldLanguages).ToList();

            foreach (string language in toRemove)
            {
                _manualPackagingInfo.DefaultLanguagePackageIds.RemoveFirst(i=>i.Language==language);
            }

            foreach (string language in toAdd)
            {
                _manualPackagingInfo.DefaultLanguagePackageIds.Add(new LanguagePackageIds(language));
            }            
        }

        internal void FreezeSelectedPackages()
        {
            using (new UndoManager.EventGroupScope())
            {
                // Take each selected package resulting items and put them in the adding items
                foreach (PackageViewModel package in Packages.Where(p => p.IsSelected))
                {
                    List<ContentItemViewModelReference> items = package.ResultingItems.ToList();

                    package.Package.Items.RemoveAll();

                    foreach (ContentItemViewModelReference item in items)
                    {
                        PackageContentItem packageItem = new PackageContentItem( item.ContentItem.Id, item.ContentItem.Language, item.ContentItem.FileName);
                        switch (item.ContentItem.FileType)
                        {
                            case FileType.SoundBank:
                                packageItem.InclusionMode = InclusionMode.Bank;
                                break;
                            case FileType.StreamedFile:
                                packageItem.InclusionMode = InclusionMode.Streams;
                                break;
                            case FileType.ExternalSource:
                                packageItem.InclusionMode = InclusionMode.External; 
                                break;
                            case FileType.LooseMedia:
                                packageItem.InclusionMode = InclusionMode.LooseMedia;
                                break;
                        }
                        
                        package.Package.Items.Add(packageItem);
                    }
                    // Then ensure no assignation rule uses those packages
                    RemovePackageFromAssignations(package.Id);
                }
            }
        }

        internal void CreateLanguagePackages()
        {
            using (new UndoManager.EventGroupScope())
            {
                // Build packages for every RSX stream file
                IEnumerable<ContentItemViewModel> items = ProjectViewModel.Current.ContentItems;

                IEnumerable<string> languages = items.Select(i => i.Language).Distinct();

                foreach (string language in languages.Where(l => l != "SFX"))
                {
                    string packageName = Path.Combine(language, "Default.pck");
                    if (!Packages.Any(p => p.Name == packageName))
                    {
                        Package package = new Package() { Name = packageName };
                        _manualPackagingInfo.Packages.Add(package);

                        // Assign
                        DefaultLanguagePackageIds.SingleOrDefault(l => l.Language == language).BankPackageId = package.Id;
                        DefaultLanguagePackageIds.SingleOrDefault(l => l.Language == language).StreamPackageId = package.Id;
                        DefaultLanguagePackageIds.SingleOrDefault(l => l.Language == language).LooseMediaPackageId = package.Id;
                    }
                }
            }
        }
    }
}
