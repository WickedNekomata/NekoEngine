using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Xml.Linq;

namespace FilePackager.Persist
{
    [MigrationStep]
    class Migrate2to3 : IMigrationStep
    {
        public int InputVersion 
        {
            get { return 2; } 
        }

        public void Migrate(XDocument document)
        {
            // Add the Output directory attribute
            document.Root.SetAttributeValue("OutputDirectory","");
        }
    }
}
