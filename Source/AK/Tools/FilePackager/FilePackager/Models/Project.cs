using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using FilePackager.Base;

namespace FilePackager.Models
{
	public class Project : ModelBase
	{
		private string _filePath = string.Empty;
		private string _infoFilePath = string.Empty;
        private string _outputDirectory = string.Empty;
		
		private ManualPackagingInfo _manualPackagingInfo;

		/// <summary>
		/// Gets or sets the file path for the project file.
		/// </summary>
		/// <value>The file path.</value>
		public string FilePath 
		{
			get { return _filePath; }
			set { SetValue(ref _filePath, value, "FilePath"); }
		}
		
		public string InfoFilePath
		{
			get { return _infoFilePath; }
			set 
            {
                // We don't check if the value is different and we always notify
                // because the path could be the same but the content different
                _infoFilePath = value;
                OnPropertyChanged("InfoFilePath");
            }
		}

        public string OutputDirectory
        {
            get { return _outputDirectory; }
            set { SetValue(ref _outputDirectory, value, "OutputDirectory"); }
        }
	
		public ManualPackagingInfo ManualPackagingInfo
		{
			get { return _manualPackagingInfo; }
		}

		public Project()
		{
			_manualPackagingInfo = new ManualPackagingInfo();
		}
	}
}
