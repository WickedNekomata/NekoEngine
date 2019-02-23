using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Collections;
using System.Security.Cryptography;
using System.Runtime.InteropServices;
using System.IO;
using System.Diagnostics;

namespace FilePackager.Base
{
	public static class Extensions
	{
		/// <summary>
		/// Adds the range of items to a list of a specific type
		/// </summary>
		/// <typeparam name="T">The type of items in the list.</typeparam>
		/// <param name="list">The list to add to.</param>
		/// <param name="enumerable">The items to add to the list.</param>
		public static void AddRange<T>(this IList<T> list, IEnumerable<T> enumerable)
		{
			foreach (T obj in enumerable)
				list.Add(obj);
		}

        /// <summary>
        /// Remove a range of items from a list of a specific type
        /// </summary>
        /// <typeparam name="T">The type of items in the list.</typeparam>
        /// <param name="list">The list to remove from.</param>
        /// <param name="enumerable">The items to remove from the list.</param>
        public static void RemoveRange<T>(this IList<T> list, IEnumerable<T> enumerable)
        {
            foreach (T obj in enumerable.ToList())
                list.Remove(obj);
        }

        /// <summary>
        /// Adds the range of items to a list of a specific type
        /// </summary>
        /// <typeparam name="T">The type of items in the list.</typeparam>
        /// <param name="list">The list to add to.</param>
        /// <param name="enumerable">The items to add to the list.</param>
        public static void AddRange(this IList list, IEnumerable enumerable)
        {
            foreach (object obj in enumerable)
                list.Add(obj);
        }

        /// <summary>
        /// Adds the range of items to a list of a specific type
        /// </summary>
        /// <typeparam name="T">The type of items in the list.</typeparam>
        /// <param name="list">The list to add to.</param>
        /// <param name="enumerable">The items to add to the list.</param>
        public static void InsertRange<T>(this IList<T> list, int index, IEnumerable<T> enumerable)
        {
            foreach (T obj in enumerable)
                list.Insert(index++, obj);
        }

		/// <summary>
		/// Removes all items in a list by calling Remove on each item.
		/// This function does not use Clear().  
		/// 
		/// This is useful in cases when INotifyCollectionChanged observers do not implement the 
		/// Clear action, but only the Add and Remove.
		/// </summary>
		/// <param name="list">The target list.</param>
		public static void RemoveAll(this IList list)
		{
			while (list.Count > 0)
			{
				list.RemoveAt(list.Count - 1);
			}
		}

        /// <summary>
        /// Returns the Index of the first item that match the predicate in the sequence.
        /// </summary>
        /// <typeparam name="TSource">The type of the source.</typeparam>
        /// <param name="items">The sequence.</param>
        /// <param name="predicate">The predicate.</param>
        /// <remarks>Throw an <exception cref="ArgumentException"/> if the predicate does not find any matching item</remarks>
        /// <returns>index of the first matching item in the sequence</returns>
        public static int IndexOf<TSource>(this IEnumerable<TSource> items, Func<TSource, bool> predicate)
        {
            int index = 0;
            foreach (TSource item in items)
            {
                if (predicate(item))
                {
                    return index;
                }
                ++index;
            }
            
            // Not found
            throw new ArgumentException("IndexOf did not found any item");
        }

        /// <summary>
        /// Appends a single item to the end of an enumerable source.
        /// </summary>
        /// <typeparam name="TSource">The type of the source.</typeparam>
        /// <param name="source">The enumerable source.</param>
        /// <param name="obj">The object to append.</param>
        /// <returns>The modified enumeration.</returns>
        public static IEnumerable<TSource> Append<TSource>(this IEnumerable<TSource> source,
            TSource obj)
        {
            return source.Concat(Enumerable.Repeat(obj, 1));
        }

        /// <summary>
        /// Prepends a single item to the front of an enumerable source.
        /// </summary>
        /// <typeparam name="TSource">The type of the source.</typeparam>
        /// <param name="source">The enumerable source.</param>
        /// <param name="obj">The object to prepend.</param>
        /// <returns>The modified enumeration.</returns>
        public static IEnumerable<TSource> Prepend<TSource>(this IEnumerable<TSource> source,
            TSource obj)
        {
            return Enumerable.Repeat(obj, 1).Concat(source);
        }

        /// <summary>
        /// Removes the first item matching the predicate.
        /// </summary>
        /// <typeparam name="TSource">The type of item in the target list.</typeparam>
        /// <param name="list">The target list.</param>
        /// <param name="predicate">The predicate.</param>
        /// <returns>Returns true if a matching item was found</returns>
        public static bool RemoveFirst<TSource>(this IList<TSource> list,
            Func<TSource, bool> predicate)
        {
            bool bRemoved = false;

            for (int i = 0; bRemoved == false && i < list.Count; i++)
            {
                if (predicate(list[i]))
                {
                    list.RemoveAt(i);
                    bRemoved = true;
                }
            }

            return bRemoved;
        }

        public static String ToString(this IEnumerable list, String delimiter)
        {
            return list.Cast<object>().ToString(delimiter, item => item.ToString());
        }

        public static String ToString<TSource>(this IEnumerable<TSource> list, String delimiter)
        {
            return list.ToString(delimiter, item => item.ToString());
        }

        public static String ToString<TSource>(this IEnumerable<TSource> list, String delimiter,
            Func<TSource, String> predicate)
        {
            if (list == null)
                return String.Empty;

            if (delimiter == null)
                throw new ArgumentNullException("delimiter");

            StringBuilder sb = new StringBuilder();

            IEnumerator<TSource> listEnum = list.GetEnumerator();
            
            if (listEnum.MoveNext())
            {
                for ( ; ; )
                {
                    if (listEnum.Current == null)
                        continue;

                    sb.Append(predicate(listEnum.Current));

                    if (!listEnum.MoveNext())
                        break;

                    sb.Append(delimiter);
                }
            }

            return sb.ToString();
        }

        /// <summary>
        /// Execute the specified action for each item.
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <param name="source">The source.</param>
        /// <param name="action">The action to execute.</param>
        /// <returns></returns>
        public static IEnumerable<T> ForEach<T>(
            this IEnumerable<T> source,
            Action<T> action)
        {
            foreach (T element in source)
                action(element);
            return source;
        }

        /// <summary>
        /// Concat 2 lists, you can pass null pointers
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <param name="source">The source.</param>
        /// <param name="enumerable">The enumerable.</param>
        /// <returns></returns>
        public static IEnumerable<T> SafeConcat<T>(this IEnumerable<T> source, IEnumerable<T> enumerable)
        {
            if (source != null && enumerable != null)
                return source.Concat(enumerable);

            if (source != null)
                return source;

            if (enumerable != null)
                return enumerable;

            // Return empty array
            return new T[0];
        }

        /// <summary>
        /// Concat 2 lists, you can pass null pointers
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <param name="source">The source.</param>
        /// <param name="enumerable">The enumerable.</param>
        /// <returns></returns>
        public static IEnumerable<T> SafeConcat<T>(this IEnumerable source, IEnumerable enumerable)
        {
            if (source != null && enumerable != null)
                return source.Cast<T>().Concat(enumerable.Cast<T>());

            if (source != null)
                return source.Cast<T>();

            if (enumerable != null)
                return enumerable.Cast<T>();

            // Return empty array
            return new T[0];
        }

        /// <summary>
        /// Counts the items of a IEnumerable.  
        /// </summary>
        /// <remarks>Only use when the Generic type is unknown</remarks>
        /// <param name="source">The source.</param>
        /// <returns>The number of items in the IEnumerable</returns>
        public static int CountNonGeneric(this IEnumerable source)
        {
            if (source == null)
                throw new ArgumentNullException("source is null");

            // When we know it is a ICollection, use the Count of the collection
            ICollection collection = source as ICollection;
            if (collection != null)
                return collection.Count;

            // Count the items
            int num = 0;
            IEnumerator enumerator = source.GetEnumerator();
            while (enumerator.MoveNext())
            {
                num++;
            }

            return num;
        }

        /// <summary>
        /// Projects each element of a sequence into a new form.
        /// </summary>
        /// <typeparam name="TSource">The type of the source.  If null, the function returns an empty sequence.</typeparam>
        /// <typeparam name="TResult">The type of the result.</typeparam>
        /// <param name="source">A sequence of values to invoke a transform function on.</param>
        /// <param name="selector">A transform function to apply to each element.</param>
        /// <returns>An IEnumerable<> whose elements are the result of invoking the transform function on each element of source.</returns>
        public static IEnumerable<TResult> SafeSelect<TSource, TResult>(this IEnumerable<TSource> source, Func<TSource, TResult> selector)
        {
            if (source != null)
                return source.Select(selector);
            return new TResult[0];
        }

        public static string PathRelativePathTo( 
            string from,
            bool fromIsDirectory,
            string to,
            bool toIsDirectory )
        {
            StringBuilder builder = new StringBuilder(260);
            bool ret = PathRelativePathTo( 
                builder, 
                from, fromIsDirectory ? FileAttributes.Directory : FileAttributes.Normal,
                to, toIsDirectory ? FileAttributes.Directory : FileAttributes.Normal );

            return ret ? builder.ToString() : to;
        }

        public static string PathCanonicalize(string path)
        {
            StringBuilder builder = new StringBuilder(260);
            if (!PathCanonicalize(builder, path))
                return path;
            return builder.ToString();
        }
            
        [DllImport("shlwapi.dll", CharSet = CharSet.Auto)]
        private static extern bool PathRelativePathTo(
             [Out] StringBuilder pszPath,
             [In] string pszFrom,
             [In] FileAttributes dwAttrFrom,
             [In] string pszTo,
             [In] FileAttributes dwAttrTo );

        [DllImport("shlwapi", CharSet = CharSet.Auto, SetLastError = true)]
        private static extern bool PathCanonicalize(
            [Out] StringBuilder lpszDst,
            [In] string lpszSrc);
	}
}
