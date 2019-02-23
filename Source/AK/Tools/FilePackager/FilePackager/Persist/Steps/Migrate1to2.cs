using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Xml.Linq;

namespace FilePackager.Persist
{
    [MigrationStep]
    class Migrate1to2 : IMigrationStep
    {
        public int InputVersion 
        {
            get { return 1; } 
        }

        public void Migrate(XDocument document)
        {
            var packageContentItems = document.Descendants("PackageContentItem");

            foreach (XElement element in packageContentItems)
            {
                // Replace the IncludeStreams boolean by a Inclusion enum
                bool includeStreams = bool.Parse(element.Attribute("IncludeStreams").Value);

                element.Attribute("IncludeStreams").Remove();

                element.SetAttributeValue("InclusionMode", includeStreams ? "BankAndStreams" : "Bank");
            }
        }
    }
}
