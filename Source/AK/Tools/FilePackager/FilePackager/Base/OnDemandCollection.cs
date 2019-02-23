using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Collections.Specialized;
using System.Collections.ObjectModel;
using System.Collections;

namespace FilePackager.Base
{
    /// <summary>
    /// The OnDemandCollection is a observable collection.  (For WPF binding)
    /// The OnDemandCollection is created empty by default, and will only populate itself
    /// on demand, when the iterator is accessed.  
    /// 
    /// User must populate the collection in the NeedPopulateCollection event.
    /// 
    /// You can call Invalidate() to invalidate the data inside the collection.  
    /// Then it will be repopulated later, when the iterator is accessed again.
    /// </summary>
    /// <typeparam name="T">The Type inside the collection</typeparam>
    public class OnDemandCollection<T> : IEnumerable<T>, IList<T>, INotifyCollectionChanged
    {
        private ObservableCollection<T> m_collection = new ObservableCollection<T>();
        private bool m_bIsInvalid = true;

        // Be notified when the collection changes (INotifyCollectionChanged)
        public event NotifyCollectionChangedEventHandler CollectionChanged;

		// Be notified when you need to populate the collection
        public event EventHandler<EventArgs> NeedPopulateCollection;

        /// <summary>
        /// Occurs when a validation is about to begin. The validation process notifies
        /// that a repopulation is required, which generate many notifications in an
        /// unpredictable order. By listening to this event, observes can ignore a bunch
        /// of notifications.
        /// </summary>
        public event EventHandler<EventArgs> Validating;
        /// <summary>
        /// Occurs when a validation is finished. The validation process notifies
        /// that a repopulation is required, which generate many notifications in an
        /// unpredictable order. By listening to this event, observes can ignore a bunch
        /// of notifications.
        /// </summary>
        public event EventHandler<EventArgs> Validated;

        /// <summary>
        /// Initializes a new instance of the <see cref="OnDemandCollection&lt;T&gt;"/> class.
        /// </summary>
        public OnDemandCollection()
        {
            // Register for the collection changed of the inner collection
            m_collection.CollectionChanged += 
                new NotifyCollectionChangedEventHandler(Collection_CollectionChanged);
        }

        /// <summary>
        /// Handles the CollectionChanged event of the inner collection.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The 
        /// <see cref="System.Collections.Specialized.NotifyCollectionChangedEventArgs"/> instance 
        /// containing the event data.</param>
        void Collection_CollectionChanged(object sender, NotifyCollectionChangedEventArgs e)
        {
            if (CollectionChanged != null)
            {
                // Just dispatch the event
                CollectionChanged(this, e);
            }
        }

        #region IEnumerable Members

        /// <summary>
        /// Returns an enumerator that iterates through a collection.
        /// </summary>
        /// <returns>
        /// An <see cref="T:System.Collections.IEnumerator"/> object that can be used to iterate 
        /// through the collection.
        /// </returns>
        IEnumerator IEnumerable.GetEnumerator()
        {
            Validate();
            return m_collection.GetEnumerator();
        }

        #endregion

        #region IEnumerable<T> Members

        /// <summary>
        /// Returns an enumerator that iterates through the collection.
        /// </summary>
        /// <returns>
        /// A <see cref="T:System.Collections.Generic.IEnumerator`1"/> that can be used to iterate 
        /// through the collection.
        /// </returns>
        IEnumerator<T> IEnumerable<T>.GetEnumerator()
        {
            Validate();
            return m_collection.GetEnumerator();
        }

        #endregion

        /// <summary>
        /// Get the valid state of the list
        /// </summary>
        public bool IsValid 
        {
            get
            {
                return !m_bIsInvalid;
            }
        }

        /// <summary>
        /// Clears all items in the collection
        /// </summary>
        public void Clear()
        {
            m_collection.RemoveAll();
        }

        /// <summary>
        /// Add an item to the collection.  You must use this function when populating the 
        /// collection.
        /// </summary>
        /// <param name="item">The item to add.</param>
        public void Add(T item)
        {
            m_collection.Add(item);
        }

        /// <summary>
        /// Invalidate the data inside the collection.  
        /// This will cause the NeedPopulateCollection to be called again.
        /// </summary>
        public void Invalidate()
        {
            m_bIsInvalid = true;

            // Call the clear function so the users will repopulate using the GetIterator function
			//Clear();
        }

        /// <summary>
        /// Populate the collection, if the data is invalid
        /// </summary>
        public void Validate()
        {
            if (m_bIsInvalid)
            {
                if (Validating != null)
                    Validating(this, EventArgs.Empty);

                m_bIsInvalid = false;

                if (NeedPopulateCollection != null)
                    NeedPopulateCollection(this, EventArgs.Empty);

                if (Validated != null)
                    Validated(this, EventArgs.Empty);
            }
        }

        #region IList<T> Members

        /// <summary>
        /// Determines the index of a specific item in the 
        /// <see cref="T:System.Collections.Generic.IList`1"/>.
        /// </summary>
        /// <param name="item">The object to locate in the 
        /// <see cref="T:System.Collections.Generic.IList`1"/>.</param>
        /// <returns>
        /// The index of <paramref name="item"/> if found in the list; otherwise, -1.
        /// </returns>
        public int IndexOf(T item)
        {
            Validate();
            return m_collection.IndexOf(item);
        }

        /// <summary>
        /// Inserts an item to the <see cref="T:System.Collections.Generic.IList`1"/> at the 
        /// specified index.
        /// </summary>
        /// <param name="index">The zero-based index at which <paramref name="item"/> should be 
        /// inserted.</param>
        /// <param name="item">The object to insert into the 
        /// <see cref="T:System.Collections.Generic.IList`1"/>.</param>
        /// <exception cref="T:System.ArgumentOutOfRangeException">
        ///     <paramref name="index"/> is not a valid index in the 
        ///     <see cref="T:System.Collections.Generic.IList`1"/>.</exception>
        /// <exception cref="T:System.NotSupportedException">The 
        /// <see cref="T:System.Collections.Generic.IList`1"/> is read-only.</exception>
        public void Insert(int index, T item)
        {
			throw new NotSupportedException();
			//Validate();
			//m_collection.Insert(index, item);
        }

        /// <summary>
        /// Removes the <see cref="T:System.Collections.Generic.IList`1"/> item at the specified 
        /// index.
        /// </summary>
        /// <param name="index">The zero-based index of the item to remove.</param>
        /// <exception cref="T:System.ArgumentOutOfRangeException">
        ///     <paramref name="index"/> is not a valid index in the 
        ///     <see cref="T:System.Collections.Generic.IList`1"/>.</exception>
        /// <exception cref="T:System.NotSupportedException">The 
        /// <see cref="T:System.Collections.Generic.IList`1"/> is read-only.</exception>
        public void RemoveAt(int index)
        {
			throw new NotSupportedException();
			//Validate();
			//m_collection.RemoveAt(index);
        }

        /// <summary>
        /// Gets or sets the value at the provided index.
        /// </summary>
        /// <param name="index">The index.</param>
        /// <returns>The object at the index.</returns>
        public T this[int index]
        {
            get
            {
                Validate();
                return m_collection[index];
            }
            set
            {
                Validate();
                m_collection[index] = value;
            }
        }

        #endregion

        #region ICollection<T> Members
        
        /// <summary>
        /// Determines whether the <see cref="T:System.Collections.Generic.ICollection`1"/> contains
        /// a specific value.
        /// </summary>
        /// <param name="item">The object to locate in the 
        /// <see cref="T:System.Collections.Generic.ICollection`1"/>.</param>
        /// <returns>
        /// true if <paramref name="item"/> is found in the 
        /// <see cref="T:System.Collections.Generic.ICollection`1"/>; otherwise, false.
        /// </returns>
        public bool Contains(T item)
        {
            Validate();
            return m_collection.Contains(item);
        }

        /// <summary>
        /// Copies the elements of the <see cref="T:System.Collections.Generic.ICollection`1"/> to 
        /// an <see cref="T:System.Array"/>, starting at a particular <see cref="T:System.Array"/> 
        /// index.
        /// </summary>
        /// <param name="array">The one-dimensional <see cref="T:System.Array"/> that is the 
        /// destination of the elements copied from <see cref="T:System.Collections.Generic.ICollection`1"/>. The <see cref="T:System.Array"/> must have zero-based indexing.</param>
        /// <param name="arrayIndex">The zero-based index in <paramref name="array"/> at which 
        /// copying begins.</param>
        /// <exception cref="T:System.ArgumentNullException">
        ///     <paramref name="array"/> is null.</exception>
        /// <exception cref="T:System.ArgumentOutOfRangeException">
        ///     <paramref name="arrayIndex"/> is less than 0.</exception>
        /// <exception cref="T:System.ArgumentException">
        ///     <paramref name="array"/> is multidimensional.-or-<paramref name="arrayIndex"/> is
        ///     equal to or greater than the length of <paramref name="array"/>.-or-The number of elements in the source <see cref="T:System.Collections.Generic.ICollection`1"/> is greater than the available space from <paramref name="arrayIndex"/> to the end of the destination <paramref name="array"/>.-or-Type <paramref name="T"/> cannot be cast automatically to the type of the destination <paramref name="array"/>.</exception>
        public void CopyTo(T[] array, int arrayIndex)
        {
            Validate();
            m_collection.CopyTo(array, arrayIndex);
        }

        /// <summary>
        /// Gets the number of elements contained in the 
        /// <see cref="T:System.Collections.Generic.ICollection`1"/>.
        /// </summary>
        /// <value></value>
        /// <returns>The number of elements contained in the 
        /// <see cref="T:System.Collections.Generic.ICollection`1"/>.</returns>
        public int Count
        {
            get
            {
                Validate();
                return m_collection.Count;
            }
        }

        /// <summary>
        /// Gets a value indicating whether the 
        /// <see cref="T:System.Collections.Generic.ICollection`1"/> is read-only.
        /// </summary>
        /// <value></value>
        /// <returns>true if the <see cref="T:System.Collections.Generic.ICollection`1"/> is 
        /// read-only; otherwise, false.</returns>
        public bool IsReadOnly
        {
            get
            {
                return false;
            }
        }

        /// <summary>
        /// Removes the first occurrence of a specific object from the 
        /// <see cref="T:System.Collections.Generic.ICollection`1"/>.
        /// </summary>
        /// <param name="item">The object to remove from the 
        /// <see cref="T:System.Collections.Generic.ICollection`1"/>.</param>
        /// <returns>
        /// true if <paramref name="item"/> was successfully removed from the 
        /// <see cref="T:System.Collections.Generic.ICollection`1"/>; otherwise, false. This method 
        /// also returns false if <paramref name="item"/> is not found in the original 
        /// <see cref="T:System.Collections.Generic.ICollection`1"/>.
        /// </returns>
        /// <exception cref="T:System.NotSupportedException">The <
        /// see cref="T:System.Collections.Generic.ICollection`1"/> is read-only.</exception>
        public bool Remove(T item)
        {
            Validate();
            return m_collection.Remove(item);
        }

        #endregion
    }
}
