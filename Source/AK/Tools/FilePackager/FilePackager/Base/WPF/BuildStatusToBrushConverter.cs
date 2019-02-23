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
    public class BuildStatusToBrushConverter : IValueConverter
	{
		public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
		{
            BuildViewModel.Status status = (BuildViewModel.Status)value;

            if (status == BuildViewModel.Status.InProgress)
                return Brushes.White;
            if (status == BuildViewModel.Status.Stopping)
                return new SolidColorBrush(Color.FromRgb(0xFA, 0xE2, 0x41));
            if (status == BuildViewModel.Status.Finished)
                return new SolidColorBrush(Color.FromRgb(0xD4, 0xD4, 0xD4));
            if (status == BuildViewModel.Status.Stopped)
                return new SolidColorBrush(Color.FromRgb(0xE9, 0x7C, 0x7C));

            return null;
		}

		public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
		{
			throw new NotSupportedException();
		}
	}}
