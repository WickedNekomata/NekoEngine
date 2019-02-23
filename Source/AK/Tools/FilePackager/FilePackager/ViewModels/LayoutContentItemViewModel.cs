using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using FilePackager.Models;

namespace FilePackager.ViewModels
{
    public class LayoutContentItemViewModel : ContentItemBaseViewModel
    {
        private PackageViewModel _package;

        public LayoutContentItemViewModel(ContentItemBase contentItemBase, PackageViewModel package)
            :base(contentItemBase)
		{
            _package = package;
		}

        public bool IsMissingFromPackage
        {
            get
            {
                // Placeholder for the remaining items
                if (Id == ContentItemBase.PlaceHolderId)
                    return false;

                // If it is missing from the project, it's not really missing from package
                if (IsMissing)
                    return false;

                ContentItemViewModel contentItem = ProjectViewModel.Current.GetContentItem(Id,Language);
                return !ProjectViewModel.Current.ReferenceManager.HasReferencesToPackage(contentItem, _package);
            }
        }

        public void OnIsInPackageChanged()
        {
            OnPropertyChanged("IsMissingFromPackage");
        }
    }
}
