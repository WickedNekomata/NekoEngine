using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.ComponentModel;
using System.Reflection;

namespace FilePackager.Base
{
	public class ModelBase
	{
		public event PropertyChangedEventHandler PropertyChanged;

		protected void OnPropertyChanged(string propertyName)
		{
			PropertyChangedEventHandler handler = PropertyChanged;

			if (handler != null)
			{
				handler(this, new PropertyChangedEventArgs(propertyName));
			}
		}

		protected void SetValue<T>(ref T target, T value, string propertyName)
		{
#if DEBUG
            // Verify the property name
            PropertyInfo pi = GetType().GetProperty(propertyName);
            if (pi == null || pi.GetGetMethod() == null)
                throw new ArgumentException();
#endif
			if (!object.Equals(target, value))
			{
				target = value;
				OnPropertyChanged(propertyName);
			}
		}
	}
}
