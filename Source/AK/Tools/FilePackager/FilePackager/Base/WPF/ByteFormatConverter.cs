using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Data;

namespace FilePackager.Base.WPF
{
    public class ByteFormatConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            long bytes = System.Convert.ToInt64(value);

            string unit;
            string format = "{0:0.0} {1}";
            double divLevel = 1.0;

            if( bytes < 100 )
            {
                unit = Properties.Resources.FormatByteSize;
                divLevel = 1.0;
                format = "{0:0.} {1}";
            }
            else if( bytes < 512000 )
            {
                unit = Properties.Resources.FormatKiloByteSize;
                divLevel = 1024.0; // 2^10
            }
            else if( bytes < 1073741824 )
            {
                unit = Properties.Resources.FormatMegaByteSize;
                divLevel = 1048576.0; // 2^20
            }
            else if( bytes < 1099511627776 )
            {
                unit = Properties.Resources.FormatGigaByteSize;
                divLevel = 1073741824.0; // 2^30
            }
            else
            {
                unit = Properties.Resources.FormatTeraByteSize;
                divLevel = 1099511627776.0; // 2^40
            }

            return string.Format( format, (double)bytes / divLevel, unit );
        }

        public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }
}
