using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace FilePackager.Base
{
    public static class NameHelpers
    {
        /// <summary>
        /// Finds a name of the unique from the list passed.  The base name must contain a single string argument: {0}
        /// </summary>
        /// <param name="names">The names.</param>
        /// <param name="baseName">The base name, ex: "New Name {0}".</param>
        /// <param name="stringComparison">The string comparison method.</param>
        /// <returns>
        /// a new name that is not present in the 'names'
        /// </returns>
        public static string FindUniqueName(IEnumerable<string> names, string baseName, StringComparison stringComparison)
        {
			string name = string.Empty;

			for (int i = 0; ; i++)
			{
                name = string.Format(baseName, i);
                if (!names.Any(n => n.Equals(name, stringComparison)))
				{
					break;
				}
			}

            return name;
        }

        /// <summary>
        /// Determines whether the specified name is duplicated in the names specified.
        /// </summary>
        /// <param name="name">The name.</param>
        /// <param name="names">The names.</param>
        /// <param name="stringComparison">The string comparison method.</param>
        /// <returns>
        /// 	<c>true</c> if the specified name is duplicated; otherwise, <c>false</c>.
        /// </returns>
        internal static bool IsDuplicated(string name, IEnumerable<string> names, StringComparison stringComparison)
        {
            return names.Where(n => n.Equals(name, stringComparison)).Count() > 1;
        }
    }
}
