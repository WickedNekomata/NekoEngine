using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Controls;
using System.Windows;

namespace FilePackager.Base.WPF
{
    public class GridViewColumnVisibilityManager
    {
        static private Dictionary<ListView, IEnumerable<GridViewColumn>> _columns = new Dictionary<ListView, IEnumerable<GridViewColumn>>();
        static private Dictionary<GridViewColumn, ListView> _listViewPerColumns = new Dictionary<GridViewColumn, ListView>();

        static void UpdateListView(ListView listView)
        {
            GridView gridView = listView.View as GridView;
            if (gridView == null || gridView.Columns == null) 
                return;

            // Backup columns
            IEnumerable<GridViewColumn> columns;
            if (!_columns.TryGetValue(listView, out columns))
                _columns[listView] = columns = gridView.Columns.ToList();

            gridView.Columns.Clear();
            gridView.Columns.AddRange(columns.Where(c => GetIsVisible(c)));
        }

        public static bool GetIsVisible(DependencyObject obj)
        {
            return (bool)obj.GetValue(IsVisibleProperty);
        }

        public static void SetIsVisible(DependencyObject obj, bool value)
        {
            obj.SetValue(IsVisibleProperty, value);
        }

        public static readonly DependencyProperty IsVisibleProperty =
            DependencyProperty.RegisterAttached("IsVisible", typeof(bool), typeof(GridViewColumnVisibilityManager), new UIPropertyMetadata(true,new PropertyChangedCallback(OnIsVisibleChanged)));

        public static bool GetIsEnabled(DependencyObject obj)
        {
            return (bool)obj.GetValue(IsEnabledProperty);
        }

        public static void SetIsEnabled(DependencyObject obj, bool value)
        {
            obj.SetValue(IsEnabledProperty, value);
        }

        public static readonly DependencyProperty IsEnabledProperty =
            DependencyProperty.RegisterAttached("IsEnabled", typeof(bool), typeof(GridViewColumnVisibilityManager), new UIPropertyMetadata(false,new PropertyChangedCallback(OnIsEnabledChanged)));

        private static void OnIsEnabledChanged(DependencyObject obj, DependencyPropertyChangedEventArgs e)
        {
            ListView listView = obj as ListView;
            if (listView != null)
            {
                bool enabled = (bool)e.NewValue;
                if (enabled)
                {
                    listView.Loaded += listView_Loaded;
                    listView.Unloaded += listView_Unloaded;

                    //view.Unloaded += (sender, e2) =>
                    //{
                    //    view.Loaded -= 
                    //};
                    //view.TargetUpdated += (sender, e2) =>
                    //{
                    //    UpdateListView((ListView)sender);
                    //};
                    //view.DataContextChanged += (sender, e2) =>
                    //{
                    //    UpdateListView((ListView)sender);
                    //};
                }
            }
        }

        static void listView_Unloaded(object sender, RoutedEventArgs e)
        {
            ListView listView = sender as ListView;
            if (listView != null)
            {
                listView.Loaded -= listView_Loaded;
                listView.Unloaded -= listView_Unloaded;
            }

            Term(listView);
            
        }

        private static void Term(ListView listView)
        {
            IEnumerable<GridViewColumn> columns;
            if (!_columns.TryGetValue(listView, out columns))
            {
                columns.ForEach(c => _listViewPerColumns.Remove(c));
                _columns.Remove(listView);
            }
        }

        static void listView_Loaded(object sender, RoutedEventArgs e)
        {
            ListView listView = (ListView)sender;
            Init(listView);
            UpdateListView(listView);
        }

        private static void Init(ListView listView)
        {
            GridView gridView = listView.View as GridView;
            if (gridView == null || gridView.Columns == null) 
                return;

            // Backup columns
            foreach (GridViewColumn column in gridView.Columns)
            {
                _listViewPerColumns[column] = listView;
            }
        }
        private static void OnIsVisibleChanged(DependencyObject obj, DependencyPropertyChangedEventArgs e)
        {
            GridViewColumn column = obj as GridViewColumn;
            if (column != null)
            {
                ListView listView;
                if (_listViewPerColumns.TryGetValue( column, out listView))
                {
                    UpdateListView(listView);
                }
            }
        }
    }
}
