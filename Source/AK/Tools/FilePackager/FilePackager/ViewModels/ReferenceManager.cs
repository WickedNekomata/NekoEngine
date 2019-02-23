using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using FilePackager.Base;

namespace FilePackager.ViewModels
{
    public class ContentItemViewModelReference
    {
        public ContentItemViewModel ContentItem { get; set; }
        public bool IsExplicit { get; set; }
    }

	internal class ReferenceManager
	{
		internal class ReferenceArgs : EventArgs
		{
            public ContentItemViewModelReference ContentItem { get; set; }
			public PackageViewModel Package { get; set; }
		}

		internal event EventHandler<ReferenceArgs> ReferenceAdded;
		internal event EventHandler<ReferenceArgs> ReferenceRemoved;

        public class PackageReference
        {
            public PackageViewModel Package { get; set; }
            public bool IsExplicit { get; set; }
        }

        private Dictionary<ContentItemViewModel, List<PackageReference>> _references = new Dictionary<ContentItemViewModel, List<PackageReference>>();

        internal void AddReference(ContentItemViewModelReference contentItem, PackageViewModel package)
		{
            List<PackageReference> packages;
			if (!_references.TryGetValue(contentItem.ContentItem, out packages))
			{
                packages = new List<PackageReference>();
				_references.Add(contentItem.ContentItem, packages);
			}

            packages.Add(new PackageReference { IsExplicit = contentItem.IsExplicit, Package = package });

			// Notify
			if (ReferenceAdded != null)
			{
				ReferenceAdded(this, new ReferenceArgs() { ContentItem = contentItem, Package = package });
			}
		}

        internal void RemoveReference(ContentItemViewModelReference contentItem, PackageViewModel package)
		{
            List<PackageReference> packages;
			if (!_references.TryGetValue(contentItem.ContentItem, out packages))
				throw new InvalidOperationException();

			if (!packages.RemoveFirst(p => p.Package == package))
				throw new InvalidOperationException();

			// Remove empty lists
			if (packages.Count == 0)
			{
				_references.Remove(contentItem.ContentItem);
			}
		
			// Notify
			if (ReferenceRemoved != null)
			{
				ReferenceRemoved(this, new ReferenceArgs() { ContentItem = contentItem, Package = package });
			}
		}

		/// <summary>
		/// Gets the references: a list of packages.
		/// </summary>
		/// <param name="contentItem">The content item is being referenced.</param>
		/// <returns>The list of references. Can return a null value when no package found</returns>
        internal IEnumerable<PackageReference> GetReferences(ContentItemViewModel contentItem)
		{
            List<PackageReference> packages = null;
			_references.TryGetValue(contentItem, out packages);
			return packages;
		}

        internal bool HasExplicitReferencesToPackage(ContentItemViewModel contentItem, PackageViewModel package)
        {
            List<PackageReference> packages = null;
            _references.TryGetValue(contentItem, out packages);

            return packages != null && 
                packages.Any(p => p.IsExplicit == true && p.Package == package);
        }

        internal bool HasReferencesToPackage(ContentItemViewModel contentItem, PackageViewModel package)
        {
            List<PackageReference> packages = null;
            _references.TryGetValue(contentItem, out packages);

            return packages != null &&
                packages.Any(p => p.Package == package);
        }

        internal void Clear()
        {
            _references.Clear();
        }
    }
}
