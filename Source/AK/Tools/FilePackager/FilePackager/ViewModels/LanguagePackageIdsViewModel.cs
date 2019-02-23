using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using FilePackager.Base;
using FilePackager.Models;

namespace FilePackager.ViewModels
{
    public class LanguagePackageIdsViewModel : ViewModelBase
    {
        private LanguagePackageIds _languagePackageId;

        public LanguagePackageIdsViewModel( LanguagePackageIds languagePackageId )
        {
            _languagePackageId = languagePackageId;
            RegisterModel(_languagePackageId);
        }

        public string Language
        {
            get
            {
                return _languagePackageId.Language;
            }
        }

        public Guid BankPackageId
        {
            get { return _languagePackageId.BankPackageId; }
            set { SetValue(_languagePackageId, "BankPackageId", value); }
        }

        public PackageViewModel BankPackage
        {
            get 
            { 
                return BankPackageId == Guid.Empty ?
                    ProjectViewModel.Current.ManualPackagingInfo.NonePackage :
                    ProjectViewModel.Current.ManualPackagingInfo.GetPackagePerId(BankPackageId); 
            }
            set 
            { 
                BankPackageId = (value == ProjectViewModel.Current.ManualPackagingInfo.NonePackage) || 
                    (value == null) ? Guid.Empty : value.Id; 
            }
        }
        
        public Guid StreamPackageId
        {
            get { return _languagePackageId.StreamPackageId; }
            set { SetValue(_languagePackageId, "StreamPackageId", value); }
        }

        public PackageViewModel StreamPackage
        {
            get
            {
                return StreamPackageId == Guid.Empty ?
                    ProjectViewModel.Current.ManualPackagingInfo.NonePackage :
                    ProjectViewModel.Current.ManualPackagingInfo.GetPackagePerId(StreamPackageId);
            }
            set
            {
                StreamPackageId = (value == ProjectViewModel.Current.ManualPackagingInfo.NonePackage) ||
                    (value == null) ? Guid.Empty : value.Id;
            }
        }

        public Guid LooseMediaPackageId
        {
            get { return _languagePackageId.LooseMediaPackageId; }
            set { SetValue(_languagePackageId, "LooseMediaPackageId", value); }
        }

        public PackageViewModel LooseMediaPackage
        {
            get
            {
                return LooseMediaPackageId == Guid.Empty ?
                    ProjectViewModel.Current.ManualPackagingInfo.NonePackage :
                    ProjectViewModel.Current.ManualPackagingInfo.GetPackagePerId(LooseMediaPackageId);
            }
            set
            {
                LooseMediaPackageId = (value == ProjectViewModel.Current.ManualPackagingInfo.NonePackage) ||
                    (value == null) ? Guid.Empty : value.Id;
            }
        }

        protected override void OnModelPropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            base.OnModelPropertyChanged(sender, e);

            if (e.PropertyName == "StreamPackageId")
			{
                OnPropertyChanged("StreamPackage");

                ProjectViewModel.Current.ManualPackagingInfo.InvalidateResultingItems();
			}
            else if (e.PropertyName == "BankPackageId")
			{
                OnPropertyChanged("BankPackage");

                ProjectViewModel.Current.ManualPackagingInfo.InvalidateResultingItems();
			}
            else if (e.PropertyName == "LooseMediaPackageId")
            {
                OnPropertyChanged("LooseMediaPackage");

                ProjectViewModel.Current.ManualPackagingInfo.InvalidateResultingItems();
            }
        }
    }
}
