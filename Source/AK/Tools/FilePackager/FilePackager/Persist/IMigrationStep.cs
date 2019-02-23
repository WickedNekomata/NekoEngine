using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Xml.Linq;

namespace FilePackager.Persist
{
    /// <summary>
    /// All your migration steps must have this attribute
    /// </summary>
    sealed class MigrationStepAttribute : Attribute
    { 
    
    }

    interface IMigrationStep
    {
        void Migrate(XDocument document);

        int InputVersion { get; }
    }
}
