using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Data;
using System.Globalization;
using System.Reflection;

namespace FilePackager.Base
{
    /// <summary>
    /// Define a description for an enumeration.
    /// </summary>
    public class ResourcesDescriptionAttribute : Attribute
    {
        private string _resourceName;
        private string _assemblyName;

        private static Dictionary<string, string> _descriptionCache = new Dictionary<string,string>();

        /// <summary>
        /// Initializes a new instance of the <see cref="ResourcesDescriptionAttribute"/> class.
        /// </summary>
        /// <param name="resourceName">Name of the resource.</param>
        /// <param name="assemblyName">Name of the assembly.</param>
        public ResourcesDescriptionAttribute( string resourceName, string assemblyName )
        {
            _resourceName = resourceName;
            _assemblyName = assemblyName;
        }

        /// <summary>
        /// Gets the description of an enumeration.
        /// </summary>
        /// <value>The description.</value>
        public string Description
        {
            get
            {
                string description = string.Empty;

                if (!_descriptionCache.TryGetValue(Key, out description))
                {
                    System.Reflection.Assembly assembly = System.Reflection.Assembly.Load(_assemblyName);

                    // Creates the ResourceManager.
                    System.Resources.ResourceManager resourceManager = new
                       System.Resources.ResourceManager( _assemblyName + ".Properties.Resources", assembly );

                    // Retrieves the string resource
                    _descriptionCache[Key] = description = resourceManager.GetString(_resourceName);
                }

                return description;
            }
        }

        /// <summary>
        /// Gets the key for caching the description.
        /// </summary>
        /// <value>The key.</value>
        private string Key
        {
            get
            {
                return _assemblyName+_resourceName;
            }
        }

        /// <summary>
        /// Gets the description of an enumeration value.
        /// </summary>
        /// <param name="en">The en.</param>
        /// <returns></returns>
        public static string GetDescription(Enum en)
        {
            Type enumType = en.GetType();
            MemberInfo[] memInfo = enumType.GetMember(en.ToString());
            
            // Get the enum value member info (can only have 1)
            if (memInfo != null && memInfo.Length > 0)
            {
                // Obtain the resource description attribute
                object[] attrs = memInfo[0].GetCustomAttributes(typeof(ResourcesDescriptionAttribute), false);
                if (attrs != null && attrs.Length > 0)
                    return ((ResourcesDescriptionAttribute)attrs[0]).Description;
            }

            // Can't find a description, use the ToString()
            return en.ToString();
        }
    }
}
