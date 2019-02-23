using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Data;
using System.Windows.Media;
using System.Text.RegularExpressions;

namespace FilePackager.Base.WPF
{
    public class IntIsZeroToBrushConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            if( parameter.GetType() != typeof(string) )
                return null;

            if (value.GetType() == typeof(Guid) && (Guid)value == Guid.Empty)
            {
                return new SolidColorBrush(ParseColor((string)parameter));
            }

            try
            {
                UInt64 iValue = System.Convert.ToUInt64(value);
                if (iValue == 0)
                {
                    return new SolidColorBrush(ParseColor((string)parameter));
                }
            }
            catch (Exception)
            { }

            return null;
        }

        public static Color ParseColor(string rgb)
        {
            byte r = byte.Parse(rgb.Substring(0, 2), System.Globalization.NumberStyles.HexNumber);
            byte g = byte.Parse(rgb.Substring(2, 2), System.Globalization.NumberStyles.HexNumber);
            byte b = byte.Parse(rgb.Substring(4, 2), System.Globalization.NumberStyles.HexNumber);
            
            return Color.FromRgb(r, g, b);
        }

        public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }
  
}
