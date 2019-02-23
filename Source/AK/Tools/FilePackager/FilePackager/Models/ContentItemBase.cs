using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using FilePackager.Base;

namespace FilePackager.Models
{
	public class ContentItemBase : ModelBase
	{
        public static readonly UInt64 PlaceHolderId = 0;

		private UInt64 _id = 0;
        private string _language;
        private string _fileName;

        public ContentItemBase(UInt64 id, string language, string fileName)
		{
			_id = id;
            _language = language;
            _fileName = fileName;
		}

		public UInt64 Id
		{
			get { return _id; }
		}

        public string Language
        {
            get { return _language; }
        }

        public string FileName
        {
            get { return _fileName; }
        }
    }
}
