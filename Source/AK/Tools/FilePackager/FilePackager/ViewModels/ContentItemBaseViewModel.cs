using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using FilePackager.Models;
using System.Windows;
using FilePackager.Base;

namespace FilePackager.ViewModels
{
	public class ContentItemBaseViewModel : ViewModelBase
	{
        protected ContentItemBase _contentItemBase;

        public ContentItemBaseViewModel(ContentItemBase contentItemBase)
		{
            _contentItemBase = contentItemBase;
			RegisterModel(_contentItemBase);
		}

		public ContentItemBase Item
		{
			get { return _contentItemBase; }
		}

        public ContentItemViewModel ContentItem
        {
            get { return ProjectViewModel.Current.GetContentItem(_contentItemBase.Id, _contentItemBase.Language); }
        }

        public UInt64 Id
        {
            get { return _contentItemBase.Id; }
        }

		public string FileName
		{
			get 
			{ 
				// Retreive the name from the ID
                ContentItemViewModel item = ContentItem;
                if (item != null)
                {
                    return item.FileName;
                }

                if (Id == ContentItemBase.PlaceHolderId)
                {
                    // Special case for the un-layout items
                    return FilePackager.Properties.Resources.UnlayoutItemsName;
                }

                // Backup with the model (means the content item disapeared)
                return _contentItemBase.FileName;
			}
		}

		public FileType FileType
		{
			get
			{
                // Retreive the type from the ID
                ContentItemViewModel item = ContentItem;
                if (item != null)
                {
                    return item.FileType;
                }

                if (Id == ContentItemBase.PlaceHolderId)
                {
                    // Special case for the un-layout items
                    return FileType.Mixed; 
                }

                // We don't know
                return FileType.Unknown; 
			}
		}

		public string Language
		{
			get
			{
                return _contentItemBase.Language;
            }
		}

        public bool IsMissing
        {
            get
            {
                if (Id == ContentItemBase.PlaceHolderId)
                {
                    return false;
                }
                return ContentItem == null;
            }
        }

        internal void OnIsMissingChanged()
        {
            OnPropertyChanged("IsMissing");
        }
	}
}
