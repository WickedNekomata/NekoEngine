using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Reflection;

namespace FilePackager.Base
{
    class EnumValueIsVisibleAttribute : Attribute
    {
        private bool _isVisible = true;
        public EnumValueIsVisibleAttribute(bool isVisible)
        {
            _isVisible = isVisible;
        }

        public bool IsVisible
        {
            get { return _isVisible; }
        }
    }

    public static class EnumHelper
    {
        public static Array GetVisibleValues(Type enumType)
        {
            return System.Enum.GetValues(enumType).Cast<Enum>().Where(e => IsVisible(enumType,e)).ToArray();
        }

        public static bool IsVisible(Type enumType, Enum en)
        {
            MemberInfo[] memInfo = enumType.GetMember(en.ToString());

            // Get the enum value member info (can only have 1)
            if (memInfo != null && memInfo.Length > 0)
            {
                // Obtain the resource description attribute
                object[] attrs = memInfo[0].GetCustomAttributes(typeof(EnumValueIsVisibleAttribute), false);
                if (attrs != null && attrs.Length > 0)
                    return ((EnumValueIsVisibleAttribute)attrs[0]).IsVisible;
            }

            return true;
        }

    }
}
