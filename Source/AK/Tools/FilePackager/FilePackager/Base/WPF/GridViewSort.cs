using System.ComponentModel;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Data;
using System.Collections;
using System.Reflection;
using System;

namespace FilePackager.Base.WPF
{
    public static class GridViewSort
    {
        #region Attached properties

        public static ICommand GetCommand(DependencyObject obj)
        {
            return (ICommand)obj.GetValue(CommandProperty);
        }

        public static void SetCommand(DependencyObject obj, ICommand value)
        {
            obj.SetValue(CommandProperty, value);
        }

        // Using a DependencyProperty as the backing store for Command.  This enables animation, styling, binding, etc...
        public static readonly DependencyProperty CommandProperty =
            DependencyProperty.RegisterAttached(
                "Command",
                typeof(ICommand),
                typeof(GridViewSort),
                new UIPropertyMetadata(
                    null,
                    (o, e) =>
                    {
                        ItemsControl listView = o as ItemsControl;
                        if (listView != null)
                        {
                            if (!GetAutoSort(listView)) // Don't change click handler if AutoSort enabled
                            {
                                if (e.OldValue != null && e.NewValue == null)
                                {
                                    listView.RemoveHandler(GridViewColumnHeader.ClickEvent, new RoutedEventHandler(ColumnHeader_Click));
                                }
                                if (e.OldValue == null && e.NewValue != null)
                                {
                                    listView.AddHandler(GridViewColumnHeader.ClickEvent, new RoutedEventHandler(ColumnHeader_Click));
                                }
                            }
                        }
                    }
                )
            );

        public static bool GetAutoSort(DependencyObject obj)
        {
            return (bool)obj.GetValue(AutoSortProperty);
        }

        public static void SetAutoSort(DependencyObject obj, bool value)
        {
            obj.SetValue(AutoSortProperty, value);
        }

        // Using a DependencyProperty as the backing store for AutoSort.  This enables animation, styling, binding, etc...
        public static readonly DependencyProperty AutoSortProperty =
            DependencyProperty.RegisterAttached(
                "AutoSort",
                typeof(bool),
                typeof(GridViewSort),
                new UIPropertyMetadata(
                    false,
                    (o, e) =>
                    {
                        ListView listView = o as ListView;
                        if (listView != null)
                        {
                            if (GetCommand(listView) == null) // Don't change click handler if a command is set
                            {
                                bool oldValue = (bool)e.OldValue;
                                bool newValue = (bool)e.NewValue;
                                if (oldValue && !newValue)
                                {
                                    listView.RemoveHandler(GridViewColumnHeader.ClickEvent, new RoutedEventHandler(ColumnHeader_Click));
                                }
                                if (!oldValue && newValue)
                                {
                                    listView.AddHandler(GridViewColumnHeader.ClickEvent, new RoutedEventHandler(ColumnHeader_Click));
                                }
                            }
                        }
                    }
                )
            );

        public static string GetPropertyName(DependencyObject obj)
        {
            return (string)obj.GetValue(PropertyNameProperty);
        }

        public static void SetPropertyName(DependencyObject obj, string value)
        {
            obj.SetValue(PropertyNameProperty, value);
        }

        // Using a DependencyProperty as the backing store for PropertyName.  This enables animation, styling, binding, etc...
        public static readonly DependencyProperty PropertyNameProperty =
            DependencyProperty.RegisterAttached(
                "PropertyName",
                typeof(string),
                typeof(GridViewSort),
                new UIPropertyMetadata(null)
            );

        #endregion

        #region Column header click event handler

        private static void ColumnHeader_Click(object sender, RoutedEventArgs e)
        {
            GridViewColumnHeader headerClicked = e.OriginalSource as GridViewColumnHeader;
            if (headerClicked != null && headerClicked.Column!=null)
            {
                string propertyName = GetPropertyName(headerClicked.Column);
                if (!string.IsNullOrEmpty(propertyName))
                {
                    ListView listView = GetAncestor<ListView>(headerClicked);
                    if (listView != null)
                    {
                        ICommand command = GetCommand(listView);
                        if (command != null)
                        {
                            if (command.CanExecute(propertyName))
                            {
                                command.Execute(propertyName);
                            }
                        }
                        else if (GetAutoSort(listView))
                        {
                            ApplySort(listView, propertyName);
                        }
                    }
                }
            }
        }

        #endregion

        #region Helper methods

        public static T GetAncestor<T>(DependencyObject reference) where T : DependencyObject
        {
            DependencyObject parent = VisualTreeHelper.GetParent(reference);
            while (!(parent is T))
            {
                parent = VisualTreeHelper.GetParent(parent);
            }
            if (parent != null)
                return (T)parent;
            else
                return null;
        }

        public static void ApplySort(ListView listView, string propertyName)
        {
            ListCollectionView collectionView = CollectionViewSource.GetDefaultView(listView.ItemsSource) as ListCollectionView;

            // Sort is not supported for non-list (For example if the ItemsSource is IEnumerable)
            if (collectionView == null)
                return;

            PropertyNaturalComparer comparer = new PropertyNaturalComparer(propertyName);
            if (collectionView.CustomSort != null)
            {
                if (((PropertyNaturalComparer)collectionView.CustomSort).IsAscending)
                    comparer.IsAscending = false;
            }

            collectionView.CustomSort = comparer;
        }

        /// <summary>
        /// Compare two properties.  When they are strings, it uses the natural comparer
        /// </summary>
        private class PropertyNaturalComparer : IComparer
        {
            private string _propertyName;
            private bool _isAscending = true;

            public PropertyNaturalComparer(string propertyName)
            {
                _propertyName = propertyName;
            }

            public bool IsAscending
            {
                get { return _isAscending; }
                set { _isAscending = value; }
            }

            public int Compare(object x, object y)
            {
                int result = _Compare(x, y);
                if (!_isAscending)
                {
                    // Reverse results
                    result *= -1;
                }
                return result;
            }

            private int _Compare(object x, object y)
            {
                // Obtain the value for both object
                object xValue = GetValue(x);
                object yValue = GetValue(y);

                if (xValue == null && yValue == null)
                    return 0;

                if (xValue == null)
                    return +1;

                if (yValue == null)
                    return -1;

                // If the two objects are string, use the Natural string compare
                string xString = xValue as string;
                string yString = yValue as string;
                if (xString != null && yString != null)
                {
                    return xString.NaturalCompareTo(yString);
                }

                // Use the standard comparaison function
                IComparable xComparable = xValue as IComparable;
                if (xComparable == null)
                    return -1;

                return xComparable.CompareTo(yValue);
            }

            /// <summary>
            /// Gets the value recursively.  It supports "PropertyA.PropertyB.PropertyC".
            /// </summary>
            /// <param name="obj">The object on which the value is retreived.</param>
            /// <returns></returns>
            private object GetValue(object obj)
            {
                object currentObj = obj;

                string[] propertyNames =_propertyName.Split('.');

                foreach (string propertyName in propertyNames)
                {
                    // Special case for Count property, which is not a property
                    IEnumerable enumerable = currentObj as IEnumerable;
                    if (propertyName == "Count" && enumerable != null)
                    {
                        return enumerable.CountNonGeneric();
                    }

                    // Obtain the get() function by reflection
                    PropertyInfo propertyInfo =
                        currentObj.GetType().GetProperty(propertyName, BindingFlags.Instance | BindingFlags.NonPublic | BindingFlags.Public | BindingFlags.FlattenHierarchy);

                    if (propertyInfo == null)
                        return null;

                    // Call the get() function
                    currentObj = propertyInfo.GetValue(currentObj, null);

                    if (currentObj == null)
                        return null;
                }

                return currentObj;
            }
        }



        #endregion
    }
}