using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using FilePackager.Packaging;
using FilePackager.ViewModels;

namespace FilePackager
{
    public interface IApplication
    {
        void ShowAboutView();
        void StartBuild( FilePackageGenerator.Context context );
        void EditPackageLayout(PackageViewModel package);
        void ShowMessage(string message, Severity severity);

        bool IsGenerateMode { get; }

        /// <summary>
        /// The implementer should checks if migration is possible 
        /// and throw an exception if not possible.
        /// </summary>
        /// <returns></returns>
        void CheckMigrate();
    }
}
