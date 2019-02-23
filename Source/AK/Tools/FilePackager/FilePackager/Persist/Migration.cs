using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Xml.Linq;
using FilePackager.ViewModels;
using System.IO;
using System.Reflection;
using FilePackager.Base;

namespace FilePackager.Persist
{
    public static class Migration
    {
        private static int _currentVersion = 7;

        private static IEnumerable<IMigrationStep> _migrationSteps;

        static Migration()
        {
            // Obtain the migration steps by reflection
            // Look for steps that have the [MigrationStep] attribute
            // Create instances of the migration steps objects
            // Order by input version
            _migrationSteps = Assembly.GetExecutingAssembly().GetTypes()
                .Where(t => t.GetCustomAttributes(typeof(MigrationStepAttribute), false).Length > 0)
                .Select(t => (IMigrationStep)Activator.CreateInstance(t))
                .OrderBy(s => s.InputVersion);
        }

        public static void Migrate(string file)
        {
            XDocument doc = XDocument.Load(file);

            int docVersion = int.Parse(doc.Root.Attribute("Version").Value);

            if (docVersion > _currentVersion)
                throw new NotSupportedException("The version of the File Packager Project is more recent than the current version supported.");

            if (docVersion < _currentVersion)
            {
                // Make sure we can do the migration
                MainViewModel.Instance.Application.CheckMigrate();

                // Backup the old project
                DateTime now = DateTime.Now;
                string date = string.Format("{0:d}-{1:d}-{2:d}.{3:d}-{4:d}-{5:d}", new object[] { now.Year, now.Month, now.Day, now.Hour, now.Minute, now.Second });
                string backup = file + ".migration." + date + ".backup";
                File.Copy(file, backup);

                // Select only the steps required
                _migrationSteps = _migrationSteps.Where(s => s.InputVersion >= docVersion);

                // Do the migration
                _migrationSteps.ForEach(step => step.Migrate(doc));

                // Update the version
                doc.Root.Attribute("Version").SetValue(CurrentVersion);

                // Save
                doc.Save(file);
            }        
        }

        public static int CurrentVersion
        {
            get { return _currentVersion; }
        }
    }
}
 