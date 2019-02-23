using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Data;
using System.Collections.ObjectModel;
using System.Windows.Controls;

namespace FilePackager.Base.WPF
{
    public class ValidationErrorGetErrorMessageConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            string sb = string.Empty;

            foreach (ValidationError error in (ReadOnlyObservableCollection<ValidationError>)value)
            {
                if (error.Exception == null || error.Exception.InnerException == null)
                {
                    sb += error.ErrorContent.ToString();
                }
                else
                {
                    sb += error.Exception.InnerException.Message;
                }
            }

            return sb;
        }

        public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }

}
