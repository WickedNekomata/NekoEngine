using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using FilePackager.Base;

namespace FilePackager.ViewModels
{
    public class AboutViewModel
    {
        public static string ApplicationName
        {
            get
            {
                return string.Format(Properties.Resources.ApplicationName, AK.Wwise.Version.VersionName);
            }
        }
        public static string Build
        {
            get
            {
                return string.Format(Properties.Resources.BuildNumber, AK.Wwise.Version.Build.ToString());
            }
        }
        public static string CopyrightNotice
        {
            get
            {
                return AK.Wwise.Version.CopyrightNotice;
            }
        }
    }
}
