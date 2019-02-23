using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Reflection;
using System.IO;

namespace FilePackager.Base.WPF
{
    /// <summary>
    /// Help management
    /// </summary>
    static public class ChmHelp
    {
        public static int GetTopicId(DependencyObject obj)
        {
            return (int)obj.GetValue(TopicIdProperty);
        }

        public static void SetTopicId(DependencyObject obj, int value)
        {
            obj.SetValue(TopicIdProperty, value);
        }

        /// <summary>
        /// Set the Help Topic ID from XAML.  Will automatically bind the F1 key
        /// Typical usage:
        ///    xmlns:base_wpf="clr-namespace:FilePackager.Base.WPF;assembly=FilePackager"
        ///    xmlns:wwise="clr-namespace:AK.Wwise;assembly=FilePackager"
        ///    base_wpf:ChmHelp.TopicId="{x:Static wwise:TopicAlias.FilePackagerMainWindow}"
        /// </summary>
        public static readonly DependencyProperty TopicIdProperty =
            DependencyProperty.RegisterAttached("TopicId", typeof(int), typeof(ChmHelp), new UIPropertyMetadata(0, OnTopicIdChanged));

        public static void OnTopicIdChanged(DependencyObject instance, DependencyPropertyChangedEventArgs e)
        {
            Control control = instance as Control;
            if (control != null)
            {
                if ((int)e.NewValue != 0)
                {
                    control.KeyDown += Control_KeyDown;
                    control.Unloaded += Control_Unloaded;
                }
            }
        }

        static void Control_Unloaded(object sender, RoutedEventArgs e)
        {
            Control control = sender as Control;
            if (control != null)
            {
                control.KeyDown -= Control_KeyDown;
                control.Unloaded -= Control_Unloaded;
            }
        }

        static void Control_KeyDown(object sender, System.Windows.Input.KeyEventArgs e)
        {
            if (e.Key == System.Windows.Input.Key.F1)
            {
                Show(GetTopicId((Control)sender));
                e.Handled = true;
            }
        }

        /// <summary>
        /// Shows the specified topic id.
        /// </summary>
        /// <param name="topicId">The topic id.</param>
        public static void Show(int topicId)
        {
            Show(topicId, System.Windows.Forms.HelpNavigator.TableOfContents);
        }

        /// <summary>
        /// Shows the specified help.
        /// </summary>
        /// <param name="navigator">The navigator.  
        /// Example: 
        ///  - System.Windows.Forms.HelpNavigator.Find
        ///  - System.Windows.Forms.HelpNavigator.Index</param>
        public static void Show(System.Windows.Forms.HelpNavigator navigator)
        {
            Show(0, navigator);
        }

        private static void Show(int topicId, System.Windows.Forms.HelpNavigator navigator)
        {
            string helpPath = Path.Combine(
                Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location),
                @"..\..\..\..\Help\WwiseHelp_en.chm");

            if (File.Exists(helpPath))
            {
                if (topicId != 0)
                {
                    navigator = System.Windows.Forms.HelpNavigator.TopicId;
                }

                System.Windows.Forms.Help.ShowHelp(null, helpPath, navigator, topicId != 0 ? topicId.ToString() : "");
            }
            else
            {
                MessageBox.Show(string.Format(Properties.Resources.CantFindHelpFile, helpPath));
            }
        }
    }
}
