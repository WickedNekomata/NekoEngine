using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Security;
using System.Runtime.InteropServices;

namespace FilePackager.Base
{
    [SuppressUnmanagedCodeSecurity]
    internal static class ShellApiNativeMethods
    {
        [DllImport("shlwapi.dll", CharSet = CharSet.Unicode)]
        public static extern int StrCmpLogicalW(string psz1, string psz2);
    }

    /// <summary>
    /// Natural string comparer
    /// </summary>
    public sealed class NaturalStringComparer : IComparer<string>
    {
        public int Compare(string a, string b)
        {
            return ShellApiNativeMethods.StrCmpLogicalW(a, b);
        }
    }

    public static class NaturalString
    {
        /// <summary>
        /// Extension method to do naturals string comparasion.
        /// </summary>
        /// <param name="a">A.</param>
        /// <param name="b">The b.</param>
        /// <returns></returns>
        public static int NaturalCompareTo(this string a, string b)
        { 
            return ShellApiNativeMethods.StrCmpLogicalW(a, b);
        }
    }
}
