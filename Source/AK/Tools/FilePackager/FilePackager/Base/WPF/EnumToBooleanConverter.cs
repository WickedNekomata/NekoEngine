using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Data;

namespace FilePackager.Base.WPF
{
	public class EnumToBooleanConverter : IValueConverter
	{
		public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
		{
			string ParameterString = parameter as string;
			if (ParameterString == null)
				return DependencyProperty.UnsetValue;

			if (Enum.IsDefined(value.GetType(), value) == false)
				return DependencyProperty.UnsetValue;

			object paramvalue = Enum.Parse(value.GetType(), ParameterString);
			if (paramvalue.Equals(value))
				return true;
			else
				return false;
		}

		public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
		{
			string ParameterString = parameter as string;
			if (ParameterString == null)
				return DependencyProperty.UnsetValue;

			return Enum.Parse(targetType, ParameterString);
		}
	}
}
