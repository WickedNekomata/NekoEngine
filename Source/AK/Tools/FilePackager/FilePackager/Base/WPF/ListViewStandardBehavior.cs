using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Controls;

namespace FilePackager.Base.WPF
{
    public class ListViewStandardBehavior : DependencyObject
    {
        public static bool GetHasStandardBehavior(DependencyObject obj)
        {
            return (bool)obj.GetValue(HasStandardBehaviorProperty);
        }

        public static void SetHasStandardBehavior(DependencyObject obj, bool value)
        {
            obj.SetValue(HasStandardBehaviorProperty, value);
        }

        // Using a DependencyProperty as the backing store for HasStandardBehavior.  This enables animation, styling, binding, etc...
        public static readonly DependencyProperty HasStandardBehaviorProperty =
            DependencyProperty.RegisterAttached("HasStandardBehavior", typeof(bool), typeof(ListViewStandardBehavior), new UIPropertyMetadata(false, OnHasStandardBehaviorChanged));

        public static void OnHasStandardBehaviorChanged(DependencyObject instance, DependencyPropertyChangedEventArgs e)
        {
            ListBox listBox = instance as ListBox;
            if (instance != null)
            {
                if ((bool)e.NewValue)
                {
                    listBox.MouseDown += listBox_MouseDown;
                    listBox.Unloaded += listBox_Unloaded;
                }
            }
        }

        static void listBox_Unloaded(object sender, RoutedEventArgs e)
        {
            Control control = sender as Control;
            if (control != null)
            {
                control.MouseDown -= listBox_MouseDown;
                control.Unloaded -= listBox_Unloaded;
            }
        }

        static void listBox_MouseDown(object sender, System.Windows.Input.MouseButtonEventArgs e)
        {
            ListBox listBox = (ListBox)sender;

            listBox.SelectedItems.Clear();
            listBox.Focus();

            e.Handled = true;
        }
    }
}
