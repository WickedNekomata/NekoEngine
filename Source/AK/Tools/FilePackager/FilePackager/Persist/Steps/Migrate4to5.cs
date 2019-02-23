using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Xml.Linq;
using FilePackager.Base;

namespace FilePackager.Persist
{
    [MigrationStep]
    class Migrate4to5 : IMigrationStep
    {
        public int InputVersion 
        {
            get { return 4; } 
        }

        public void Migrate(XDocument document)
        {
            // Then add empty default external package
            document.Root.Element("ManualPackagingInfo").Add( new XAttribute("UnassignedExternalsPackageId", Guid.Empty.ToString() ));
        }
    }
}
