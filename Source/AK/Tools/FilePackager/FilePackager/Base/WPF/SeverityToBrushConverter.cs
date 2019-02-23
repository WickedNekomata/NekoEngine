using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Data;
using FilePackager.ViewModels;
using System.Windows.Media;
using FilePackager.Packaging;

namespace FilePackager.Base.WPF
{
    public class SeverityToBrushConverter : IValueConverter
	{
		public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
		{
            Severity severity = (Severity)value;
            if (severity == Severity.Message)
                return Brushes.White;
            if (severity == Severity.Error)
                return new SolidColorBrush(Color.FromRgb(0xE9, 0x7C, 0x7C));

            return new SolidColorBrush(Color.FromRgb(0xFA,0xE2,0x41));
		}

		public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
		{
			throw new NotSupportedException();
		}
	}
}
