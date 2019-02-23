using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Collections.ObjectModel;
using FilePackager.Base;

namespace FilePackager.Models
{
	public class Package : ModelBase
	{
		private Guid _id;
		private string _name = string.Empty;
		private bool _splitByLanguage = true;
		private ObservableCollection<PackageContentItem> _items = new ObservableCollection<PackageContentItem>();
        private ObservableCollection<ContentItemBase> _layoutItems = new ObservableCollection<ContentItemBase>();

		public Package()
		{
			_id = Guid.NewGuid();
            _layoutItems.Add(new ContentItemBase(ContentItemBase.PlaceHolderId, "", ""));
		}

        public Package(Guid id, IEnumerable<PackageContentItem> items, IEnumerable<ContentItemBase> layoutItems)
		{
			_id = id;
			_items.AddRange(items);
            _layoutItems.AddRange(layoutItems);
		}

        public Guid Id
        {
            get { return _id; }
        }

		public string Name
		{
			get { return _name; }
			set { SetValue(ref _name, value, "Name"); }
		}

		public bool SplitByLanguage
		{
			get { return _splitByLanguage; }
			set { SetValue(ref _splitByLanguage, value, "SplitByLanguage"); }
		}

		public ObservableCollection<PackageContentItem> Items
		{
			get { return _items; }
		}

        public ObservableCollection<ContentItemBase> LayoutItems
        {
            get { return _layoutItems; }
        }
    }
}
