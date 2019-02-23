using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace FilePackager.Base
{
    class StringEqualityComparer: IEqualityComparer<string>
    {
        StringComparison _comparisonType;
        public StringEqualityComparer(StringComparison comparisonType)
        {
            _comparisonType = comparisonType;
        }

        public bool Equals(string x, string y)
        {
            return string.Equals(x, y, _comparisonType);
        }

        public int GetHashCode(string obj)
        {
            return obj.GetHashCode();
        }
    }
}
