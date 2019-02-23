using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Xml.Linq;
using FilePackager.Base;

namespace FilePackager.Persist
{
    [MigrationStep]
    class Migrate6to7 : IMigrationStep
    {
        public int InputVersion 
        {
            get { return 6; } 
        }

        public void Migrate(XDocument document)
        {
            var manualPackingInfo = document.Root.Element("ManualPackagingInfo");
            
            manualPackingInfo.Add(new XAttribute("UnassignedLooseMediaPackageId", Guid.Empty.ToString()));

            var langPackIds = manualPackingInfo.Element("DefaultLanguagePackageIds").Elements("LanguagePackageIds");

            langPackIds.ForEach(p => p.Add(new XAttribute("LooseMediaPackageId", Guid.Empty.ToString())));
        }
    }
}
