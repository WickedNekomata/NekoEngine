using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Xml.Linq;

namespace FilePackager.Persist
{
    [MigrationStep]
    class Migrate3to4 : IMigrationStep
    {
        public int InputVersion 
        {
            get { return 3; } 
        }

        public void Migrate(XDocument document)
        {
            var packages = document.Descendants("Package");

            foreach (XElement package in packages)
            { 
                // Add the .pck extention to all package names
                string oldName = package.Attribute("Name").Value;
                if (!oldName.EndsWith(".pck"))
                {
                    package.Attribute("Name").Value = oldName + ".pck";
                }
            }
        }
    }
}
