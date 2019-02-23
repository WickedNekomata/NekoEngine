using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using FilePackager.Models;
using System.Windows;
using FilePackager.Base;

namespace FilePackager.ViewModels
{
    public class PackageContentItemViewModel : ContentItemBaseViewModel
	{
        private PackageViewModel _package;

		public PackageContentItemViewModel( PackageContentItem packageContentItem, PackageViewModel package )
            : base(packageContentItem)
		{
            _package = package;

            _package.PropertyChanged += Package_PropertyChanged;
		}

        void Package_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
        }

        public override void Detach()
        {
            _package.PropertyChanged -= Package_PropertyChanged;

            base.Detach();
        }

        protected override void OnModelPropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            base.OnModelPropertyChanged(sender, e);

            if (e.PropertyName == "InclusionMode")
            {
                // Invalidate the resulting items
                ProjectViewModel.Current.ManualPackagingInfo.InvalidateResultingItems();
            }
        }

        public InclusionMode InclusionMode
		{
            get 
            { 
                if( HaveInclusionMode )
                    return (_contentItemBase as PackageContentItem).InclusionMode;

                // Implicit Inclusion Mode
                if (FileType == FileType.ExternalSource)
                    return InclusionMode.External;
                
                if (FileType == FileType.StreamedFile)
                    return InclusionMode.Streams;

                if (FileType == FileType.LooseMedia)
                    return InclusionMode.LooseMedia;

                return InclusionMode.Invalid;
            }
            set { SetValue(_contentItemBase, "InclusionMode", value); }
		}

        public bool HaveInclusionMode
        {
            get 
            {
                return FileType == FileType.SoundBank; 
            }
        }

        public bool CanIncludeExplicitly
        {
            get
            {
                InclusionMode mode = InclusionMode;

                switch (FileType)
                {
                    case FileType.SoundBank:
                        return  mode.HasFlag( InclusionMode.Bank );
                    
                    case FileType.StreamedFile:
                        return mode.HasFlag( InclusionMode.Streams );

                    case FileType.ExternalSource:
                        return mode.HasFlag( InclusionMode.External );

                    case FileType.LooseMedia:
                        return mode.HasFlag( InclusionMode.LooseMedia );
                }
                return false;
            }
        }

        static readonly ContentItemViewModel[] _noItems = new ContentItemViewModel[0];
        public IEnumerable<ContentItemViewModel> ImplicitItems
        {
            get
            {
                IEnumerable<ContentItemViewModel> items = _noItems;

                if (FileType == FileType.SoundBank )
                {
                    InclusionMode mode = InclusionMode;
                    if (mode.HasFlag(InclusionMode.Streams))
                    {
                        items = ContentItem.ReferencedStreamedFiles.Where(i => i.FileType == FileType.StreamedFile);
                    }
                    
                    if (mode.HasFlag(InclusionMode.LooseMedia))
                    {
                        items = items.Concat(ContentItem.ReferencedLooseMedia);
                    }
                }

                return items;
            }
        }
	}
}
