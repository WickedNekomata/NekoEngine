using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using FilePackager.Models;
using System.Collections.ObjectModel;
using FilePackager.Commands;
using System.Windows.Input;
using FilePackager.Undo;
using FilePackager.Base;
using FilePackager.Base.WPF;
using System.Collections;
using System.Windows;
using System.Windows.Controls;
using System.ComponentModel;
using System.IO;

namespace FilePackager.ViewModels
{
    public class PackageViewModel : ViewModelBase, IDataErrorInfo, IRenamable
	{
		private Package _package;
		private bool _isSelected = false;
		private UndoableObservableCollection<PackageContentItemViewModel, PackageContentItem> _items;
        private UndoableObservableCollection<LayoutContentItemViewModel, ContentItemBase> _layoutItems;
        private ObservableCollection<ContentItemViewModelReference> _resultingItems;
        private long _size = 0;

        private BackgroundWorker _sizeSumWorker = new BackgroundWorker();

        private ManualPackagingInfoViewModel _manualPackagingInfo;

		private DelegateCommand _removeContentItemCommand;
		private DelegateCommand _insertContentItemsCommand;
        private DelegateCommand _copyResultingItemsToClipboardCommand;
        private DelegateCommand _editLayoutCommand;
        private DelegateCommand _dropLayoutItemsCommand;
        private DelegateCommand _removeLayoutItemsCommand;
        private DelegateCommand _removeMissingItemsCommand;
        private DelegateCommand _addToLayoutCommand;
        private DelegateCommand _renameCommand;

        public event EventHandler RenameRequested;
        public event EventHandler PreUnlayoutItemsChanged;

		private PackageContentItemViewModel CreatePackageContentItemViewModel(PackageContentItem packageContentItem)
		{
			return new PackageContentItemViewModel(packageContentItem,this);
		}

        private LayoutContentItemViewModel CreateContentItemBaseViewModel(ContentItemBase packageContentItem)
		{
            return new LayoutContentItemViewModel(packageContentItem,this);
		}

        public PackageViewModel(Package package, ManualPackagingInfoViewModel manualPackagingInfo)
		{
			_package = package;
			RegisterModel(_package);

            _manualPackagingInfo = manualPackagingInfo;

			_items = new UndoableObservableCollection<PackageContentItemViewModel, PackageContentItem>(_package.Items, CreatePackageContentItemViewModel);
            _layoutItems = new UndoableObservableCollection<LayoutContentItemViewModel, ContentItemBase>(_package.LayoutItems, CreateContentItemBaseViewModel);
            _resultingItems = new ObservableCollection<ContentItemViewModelReference>();

			_items.CollectionChanged += _items_CollectionChanged;
			_resultingItems.CollectionChanged += _resultingItems_CollectionChanged;
            _layoutItems.CollectionChanged += _layoutItems_CollectionChanged;

            _sizeSumWorker.DoWork += SizeSumWorker_DoWork;
            _sizeSumWorker.RunWorkerCompleted += SizeSumWorker_RunWorkerCompleted;
            _sizeSumWorker.WorkerSupportsCancellation = true;

            PropertyChanged += PackageViewModel_PropertyChanged;

            _manualPackagingInfo.PropertyChanged += ManualPackagingInfo_PropertyChanged;
		}

        void ManualPackagingInfo_PropertyChanged(object sender, PropertyChangedEventArgs e)
        {
        }

        void PackageViewModel_PropertyChanged(object sender, PropertyChangedEventArgs e)
        {
        }

        public override void Detach()
        {
            base.Detach();

            _resultingItems.CollectionChanged -= _resultingItems_CollectionChanged;
            _items.CollectionChanged -= _items_CollectionChanged;
            _layoutItems.CollectionChanged -= _layoutItems_CollectionChanged;

            _items.Detach();
            _layoutItems.Detach();

            _sizeSumWorker.DoWork -= SizeSumWorker_DoWork;
            _sizeSumWorker.RunWorkerCompleted -= SizeSumWorker_RunWorkerCompleted;

            PropertyChanged -= PackageViewModel_PropertyChanged;

            _manualPackagingInfo.PropertyChanged -= ManualPackagingInfo_PropertyChanged;
        }

        public override IEnumerable<string> Errors
        {
            get
            {
                var errors = _items.SelectMany(i => i.Errors)
                    .SafeConcat(_layoutItems.SelectMany(i => i.Errors))
                    .SafeConcat(base.Errors);

                string packageNameError = this["Name"];

                if (packageNameError != null)
                    errors = errors.Append(
                        string.Format(Properties.Resources.InvalidPackageName, Name, packageNameError));

                return errors;
            }
        }

        void _layoutItems_CollectionChanged(object sender, System.Collections.Specialized.NotifyCollectionChangedEventArgs e)
        {
            OnUnlayoutItemsChanged();
        }

        public void OnUnlayoutItemsChanged()
        {
            // When the layout items change, the layout items change too
            if (UndoManager.Instance.IsBusy || UndoManager.Instance.HasActiveGroup)
            {
                // Wait until the last group is closed
                UndoManager.Instance.RegisterOnceToIsBusyOrGroupFinished(UndoManager_IsBusyOrGroupFinishedUpdateUnlayoutItems);
            }
            else
            {
                _OnUnlayoutItemsChanged();
            }
        }

        void UndoManager_IsBusyOrGroupFinishedUpdateUnlayoutItems(object sender, EventArgs e)
        {
            UndoManager.Instance.IsBusyOrGroupFinished -= UndoManager_IsBusyOrGroupFinishedUpdateUnlayoutItems;
            _OnUnlayoutItemsChanged();
        }

        private void _OnUnlayoutItemsChanged()
        {
            // Send the "Pre" event
            if (PreUnlayoutItemsChanged != null)
            {
                PreUnlayoutItemsChanged(this, EventArgs.Empty);
            }

            // then the PropertyChanged event
            OnPropertyChanged("UnlayoutItems");
        }

		void _resultingItems_CollectionChanged(object sender, System.Collections.Specialized.NotifyCollectionChangedEventArgs e)
		{
			switch (e.Action)
			{
				case System.Collections.Specialized.NotifyCollectionChangedAction.Add:
					// Add references
                    foreach (ContentItemViewModelReference item in e.NewItems)
					{
						ProjectViewModel.Current.ReferenceManager.AddReference(item, this);
					}
					break;

				case System.Collections.Specialized.NotifyCollectionChangedAction.Remove:
					// Remove references
                    foreach (ContentItemViewModelReference item in e.OldItems)
					{
						ProjectViewModel.Current.ReferenceManager.RemoveReference(item, this);
					}

					break;

				case System.Collections.Specialized.NotifyCollectionChangedAction.Move:
					// No action to take
					break;
				case System.Collections.Specialized.NotifyCollectionChangedAction.Replace:
				case System.Collections.Specialized.NotifyCollectionChangedAction.Reset:
				default:
					throw new NotSupportedException();
			}

            // When adding/removing items from the list, remove then from the layout
            var allItems = e.NewItems.SafeConcat<ContentItemViewModelReference>(e.OldItems);
            foreach (ContentItemViewModelReference item in allItems)
            {
                // Make sure it is not present in the layout items
                LayoutContentItemViewModel layoutItem = _layoutItems.FirstOrDefault(i => i.Id == item.ContentItem.Id && i.Language == item.ContentItem.Language);
                if (layoutItem != null)
                {
                    layoutItem.OnIsInPackageChanged();
                }
            }
		}

		void _items_CollectionChanged(object sender, System.Collections.Specialized.NotifyCollectionChangedEventArgs e)
		{
            ProjectViewModel.Current.ManualPackagingInfo.InvalidateResultingItems();

            // When the layout items change, the layout items change too
            if (UndoManager.Instance.IsBusy || UndoManager.Instance.HasActiveGroup)
            {
                // Wait until the last group is closed
                UndoManager.Instance.RegisterOnceToIsBusyOrGroupFinished(UndoManager_IsBusyOrGroupFinishedUpdateMissingItems);
            }
            else
            {
                OnMissingItemsChanged(); 
            }
		}

        void UndoManager_IsBusyOrGroupFinishedUpdateMissingItems(object sender, EventArgs e)
        {
            UndoManager.Instance.IsBusyOrGroupFinished -= UndoManager_IsBusyOrGroupFinishedUpdateMissingItems;
            OnMissingItemsChanged(); 
        }

        public void OnMissingItemsChanged()
        {
            OnPropertyChanged("MissingItems");
        }

		public Package Package
		{
			get
			{
				return _package;
			}
		}

		public Guid Id
		{
			get { return _package.Id; }
		}

		public string Name
		{
			get { return _package.Name; }
			set { SetValue(_package, "Name", value); }
		}

		public bool SplitByLanguage
		{
			get { return _package.SplitByLanguage; }
			set { SetValue(_package, "SplitByLanguage", value); }
		}

		public long Size
		{
			get 
			{
                return _size; 
			}
		}

        public uint HeaderSize
        {
            get
            {
                int externalCount = _resultingItems.Count(i=>i.ContentItem.FileType==FileType.ExternalSource);
                uint size = FilePackager.Packaging.FilePackageGenerator.BaseHeaderSize +
                    FilePackager.Packaging.StringMap.ApproximateTotalSize +
                    FilePackager.Packaging.FileLUT.GetApproximateTotalSize((uint)_resultingItems.Count - (uint)externalCount, (uint)externalCount);

                return size;
            }
        }

		public ObservableCollection<PackageContentItemViewModel> Items
		{
			get
			{
				return _items;
			}
		}

        public ObservableCollection<LayoutContentItemViewModel> LayoutItems
        {
            get
            {
                return _layoutItems;
            }
        }

        public class ContentItemComparable
        {
            public UInt64 Id;
            public string Language;
            public string FileName;

            public ContentItemComparable(UInt64 id, string language, string fileName)
            {
                Id = id;
                Language = language;
                FileName = fileName;
            }

            public override bool Equals(object obj)
            {
                ContentItemComparable other = (ContentItemComparable)obj;
                return Id == other.Id && Language == other.Language;
            }

            public override int GetHashCode()
            {
                return (Id.ToString() + Language).GetHashCode();
            }
        }

        public IList<ContentItemBaseViewModel> UnlayoutItems
        {
            get
            {
                using (new AutoCursor(Cursors.Wait))
                {
                    var all = ResultingItems.Select(i => new ContentItemComparable(i.ContentItem.Id, i.ContentItem.Language, i.ContentItem.FileName));
                    var layout = LayoutItems.Select(i => new ContentItemComparable(i.Id, i.Language, i.FileName));

                    var unlayout = all.Except(layout).Select(i => new ContentItemBaseViewModel(new ContentItemBase(i.Id, i.Language, i.FileName)));

                    return unlayout.ToList();
                }
            }
        }

        /// <summary>
        /// Gets the resolved layout items.  Remove the missing items, and replace the placeholder by
        /// the remaining items.
        /// </summary>
        /// <value>The resolved layout items.</value>
        public IEnumerable<ContentItemBaseViewModel> ResolvedLayoutItems
        {
            get
            {
                var validLayoutItems = LayoutItems.Where( i => !i.IsMissingFromPackage && !i.IsMissing);
                int index = validLayoutItems.IndexOf(i => i.Id == ContentItemBase.PlaceHolderId);

                return validLayoutItems.Cast<ContentItemBaseViewModel>()
                    .Take(index)
                    .Concat(UnlayoutItems)
                    .Concat(validLayoutItems.Cast<ContentItemBaseViewModel>().Skip(index + 1));
            }
        }

		public bool IsSelected
		{
			get { return _isSelected; }
			set { _isSelected = value; }
		}

		public ICommand RemoveContentItemCommand
		{
			get { return CommandManagerHelper.CreateOnceCommand(ref _removeContentItemCommand, RemoveContentItem, CanRemoveContentItem); }
		}

		public ICommand InsertContentItemsCommand
		{
			get { return CommandManagerHelper.CreateOnceCommand(ref _insertContentItemsCommand, InsertContentItems, CanInsertContentItems); }
		}

        public ICommand CopyResultingItemsToClipboardCommand
		{
            get { return CommandManagerHelper.CreateOnceCommand(ref _copyResultingItemsToClipboardCommand, CopyResultingItemsToClipboard, null); }
		}

        public ICommand EditLayoutCommand
		{
            get { return CommandManagerHelper.CreateOnceCommand(ref _editLayoutCommand, EditLayout, null); }
		}

        public ICommand DropLayoutItemsCommand
		{
            get { return CommandManagerHelper.CreateOnceCommand(ref _dropLayoutItemsCommand, DropLayoutItems, null); }
		}

        public ICommand RemoveLayoutItemsCommand
		{
            get { return CommandManagerHelper.CreateOnceCommand(ref _removeLayoutItemsCommand, RemoveLayoutItems, CanRemoveLayoutItems); }
		}

        public ICommand RemoveMissingItemsCommand
		{
            get { return CommandManagerHelper.CreateOnceCommand(ref _removeMissingItemsCommand, RemoveMissingItems, CanRemoveMissingItems); }
		}
        
        public ICommand AddToLayoutCommand
		{
            get { return CommandManagerHelper.CreateOnceCommand(ref _addToLayoutCommand, AddToLayout, CanAddToLayout); }
		}

        public ICommand RenameCommand
        {
            get { return CommandManagerHelper.CreateOnceCommand(ref _renameCommand, Rename, null); }
        }

        private void RemoveContentItem(object parameter)
		{
            using (new UndoManager.EventGroupScope())
            {
                // Remove selected packages
                var selection = ((IList)parameter).Cast<PackageContentItemViewModel>().Select(pci => pci.Item).ToList();
                foreach (PackageContentItem packageContentItem in selection)
                {
                    _package.Items.Remove(packageContentItem);
                }
            }
		}

        private bool CanRemoveContentItem(object parameter)
		{
            return ((IList)parameter).Count > 0;
		}

		private void InsertContentItems(object parameter)
		{
            ProjectViewModel project = ProjectViewModel.Current;

			DragDropHelper.DragDropData data = (DragDropHelper.DragDropData)parameter;

			IEnumerable<ContentItemViewModel> items = (data.Data as IList).Cast<ContentItemViewModel>();

            // Check for duplicates
            var newItems = from civm in items
                           where !project.ReferenceManager.HasExplicitReferencesToPackage(civm,this)
                           select new PackageContentItem(civm.Id, civm.Language, civm.FileName);

            using (new UndoManager.EventGroupScope())
            {
                _package.Items.AddRange(newItems);
            }
		}

		private bool CanInsertContentItems(object parameters)
		{
			return true;
		}

        private void DropLayoutItems(object parameters)
        {
            DragDropHelper.DragDropData data = (DragDropHelper.DragDropData)parameters;

            IEnumerable<ContentItemBaseViewModel> items = (data.Data as IList).Cast<ContentItemBaseViewModel>().ToList();

            using (new UndoManager.EventGroupScope())
            {
                if (data.IsReorder)
                {
                    // Find the destination
                    bool IsAtEnd = false;
                    ContentItemBase target = null;
                    if (data.IndexInserted == _package.LayoutItems.Count)
                    {
                        IsAtEnd = true;
                    }
                    else
                    {
                        while (target == null && data.IndexInserted < _package.LayoutItems.Count)
                        {
                            ContentItemBase testTarget = _package.LayoutItems[data.IndexInserted];
                            if (items.Select(i => i.Item).Contains(testTarget))
                                data.IndexInserted++;
                            else
                                target = testTarget;
                        }
                    }

                    // Remove the moved items
                    items.ForEach(i => _package.LayoutItems.Remove(i.Item));

                    // Add them to the new location
                    data.IndexInserted = IsAtEnd ? _package.LayoutItems.Count : 0;
                    if (target != null)
                    {
                        data.IndexInserted = _package.LayoutItems.IndexOf(target);
                    }
                }

                _package.LayoutItems.InsertRange(data.IndexInserted, items.Select(i => i.Item).ToList());
            }
        }

        private void RemoveLayoutItems(object parameter)
        {
            // Remove selected packages
            var selection = ((IList)parameter).Cast<ContentItemBaseViewModel>()
                .Where(i => i.Id != ContentItemBase.PlaceHolderId)      // Skip the "Unlayout items" item
                .Select(pci => pci.Item).ToList();

            using (new UndoManager.EventGroupScope())
            {
                foreach (ContentItemBase contentItemBase in selection)
                {
                    _package.LayoutItems.Remove(contentItemBase);
                }
            }
        }

        private bool CanRemoveLayoutItems(object parameter)
        {
            return ((IList)parameter).Count > 0;
        }

        private void RemoveMissingItems()
        {
            IEnumerable<LayoutContentItemViewModel> missings = _layoutItems.Where(i => i.IsMissing || i.IsMissingFromPackage).ToList();

            using (new UndoManager.EventGroupScope())
            {
                Package.LayoutItems.RemoveRange(missings.Select(i => i.Item));
            }
        }

        private bool CanRemoveMissingItems()
        {
            return _layoutItems.Any(i => i.IsMissing || i.IsMissingFromPackage);
        }

        private void AddToLayout(object parameter)
        {
            var selection = ((IList)parameter).Cast<ContentItemBaseViewModel>()
                .Select(pci => pci.Item).ToList();

            using (new UndoManager.EventGroupScope())
            {
                _package.LayoutItems.AddRange(selection);
            }
        }

        private bool CanAddToLayout(object parameter)
        {
            return ((IList)parameter).Count > 0;
        }

        public ObservableCollection<ContentItemViewModelReference> ResultingItems
		{
			get
			{
				return _resultingItems;
			}
		}

        public IEnumerable<PackageContentItemViewModel> MissingItems
        {
            get
            {
                return _items.Where(i => i.IsMissing);
            }
        }

		public string Error
		{
			get { return null; }
		}

		public string this[string name]
		{
			get
			{
				if (name == "Name")
				{
                    if (string.IsNullOrEmpty(Name.Trim()))
                        return "The name is empty.";

                    // Validate the invalid characters
                    try
                    {
                        string toValidate = _package.Name;
                        if (Path.IsPathRooted(_package.Name))
                        {
                            // Skip the root
                            toValidate = toValidate.Substring(Path.GetPathRoot(_package.Name).Length);
                        }

                        if (toValidate.IndexOfAny(Path.GetInvalidFileNameChars().Where(c => c != '\\').ToArray<char>()) != -1)
                            return "The name contains invalid characters.";
                    }
                    catch(Exception e)
                    {
                        return e.Message;
                    }

                    // Check for name duplicates
                    if (NameHelpers.IsDuplicated(
                        _package.Name,
                        ProjectViewModel.Current.ManualPackagingInfo.Packages.Select(p => p.Name),
                        StringComparison.OrdinalIgnoreCase))
                    {
                        return "Package name is duplicated.";
                    }
				}

				return null;
			}
		}

        public bool CanBuild
        {
            get
            {
                return string.IsNullOrEmpty(this["Name"]);
            }
        }

        internal void Delete()
        {
            Items.RemoveAll();
            ResultingItems.RemoveAll();
        }

        private void CopyResultingItemsToClipboard()
        {
            StringBuilder output = new StringBuilder();

            foreach (ContentItemViewModelReference item in ResultingItems)
            {
                output.Append(item.ContentItem.FileName);
                output.Append("\t");
                output.Append(item.ContentItem.FileType.ToString());
                output.Append("\t");
                output.Append(item.ContentItem.Language);
                output.Append("\t");
                output.Append(item.ContentItem.Size);
                output.Append("\r\n");
            }

            Clipboard.SetText(output.ToString());        
        }

        private void EditLayout()
        {
            MainViewModel.Instance.Application.EditPackageLayout(this);
        }

        protected override void OnModelPropertyChanged(object sender, PropertyChangedEventArgs e)
        {
            base.OnModelPropertyChanged(sender, e);

            if (e.PropertyName=="Name")
            {
                ResultingItems.ForEach(i => i.ContentItem.OnReferencesChanged());
            }
        }

        public void Rename()
        {
            if (RenameRequested != null)
            {
                RenameRequested(this, EventArgs.Empty);
            }
        }
                
        internal void OnResultingItemsChanged()
        {
            if (_sizeSumWorker.IsBusy)
                _sizeSumWorker.CancelAsync();
            else
                _sizeSumWorker.RunWorkerAsync();

            OnPropertyChanged("HeaderSize");
        }

        void SizeSumWorker_DoWork(object sender, DoWorkEventArgs e)
        {
            _size = 0;
            foreach (ContentItemViewModelReference item in ResultingItems)
            {
                _size += item.ContentItem.Size;
                if (_sizeSumWorker.CancellationPending)
                {
                    e.Cancel = true;
                    return;
                }
            }
        }

        void SizeSumWorker_RunWorkerCompleted(object sender, RunWorkerCompletedEventArgs e)
        {
            // Rerun the worker if it was cancelled
            if (e.Cancelled)
			{
                // Handle race condition. Retry 50 times at 100 ms interval.
                int tryCount = 0;

                while (_sizeSumWorker.IsBusy || tryCount < 50)
                {
                    System.Threading.Thread.Sleep(100);
                    ++tryCount;
                }

                _sizeSumWorker.RunWorkerAsync();				
			}
            else
                OnPropertyChanged("Size");
        }
    }
}
