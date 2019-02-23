using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace FilePackager.Base.WPF
{
    /// <summary>
    /// For any object that can request a rename from a view
    /// </summary>
    interface IRenamable
    {
        /// <summary>
        /// Occurs when a rename action is requested
        /// </summary>
        event EventHandler RenameRequested;
    }
}
