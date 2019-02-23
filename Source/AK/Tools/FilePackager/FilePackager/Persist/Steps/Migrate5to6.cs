using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Xml.Linq;
using FilePackager.Base;

namespace FilePackager.Persist
{
    [MigrationStep]
    class Migrate5to6 : IMigrationStep
    {
        public int InputVersion 
        {
            get { return 5; } 
        }

        public void Migrate(XDocument document)
        {
            // Then add empty default external package
            var packages = document.Root.Element("ManualPackagingInfo").Element("Packages").Elements("Package");

            packages.ForEach(p => p.SetAttributeValue("IsRSX", false));

            document.Root.Element("ManualPackagingInfo").SetAttributeValue("BlockSizeRSX", 128);
        }
    }
}
