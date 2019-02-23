using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using FilePackager.Base;

namespace FilePackager.Models
{
    public class LanguagePackageIds : ModelBase
    {
        private string _language;
        private Guid _bankPackageId = Guid.Empty;
        private Guid _streamPackageId = Guid.Empty;
        private Guid _looseMediaPackageId = Guid.Empty;

        public LanguagePackageIds(string language)
        {
            _language = language;
        }

        public Guid BankPackageId
        {
            get { return _bankPackageId; }
            set { SetValue(ref _bankPackageId, value, "BankPackageId"); }
        }
        
        public Guid StreamPackageId
        {
            get { return _streamPackageId; }
            set { SetValue(ref _streamPackageId, value, "StreamPackageId"); }
        }

        public Guid LooseMediaPackageId
        {
            get { return _looseMediaPackageId; }
            set { SetValue(ref _looseMediaPackageId, value, "LooseMediaPackageId"); }
        }

        public string Language
        {
            get
            {
                return _language;
            }
        }
    }
}
