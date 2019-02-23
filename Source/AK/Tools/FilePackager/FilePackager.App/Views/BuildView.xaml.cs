using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Shapes;
using FilePackager.ViewModels;

namespace FilePackager.Views
{
    /// <summary>
    /// Interaction logic for BuildView.xaml
    /// </summary>
    public partial class BuildView : Window
    {
        public BuildView()
        {
            InitializeComponent();
        }

        public BuildViewModel Build
        {
            get
            {
                return (DataContext as BuildViewModel);
            }
        }

        void BuildView_Finished(object sender, EventArgs e)
        {
            Close();
        }

        protected override void OnPropertyChanged(DependencyPropertyChangedEventArgs e)
        {
            base.OnPropertyChanged(e);

            if (e.Property == DataContextProperty)
            {
                if (e.OldValue != null)
                    (e.OldValue as BuildViewModel).Finished -= BuildView_Finished;
                if (e.NewValue != null)
                    (e.NewValue as BuildViewModel).Finished += BuildView_Finished;
            }
        }

        private void Window_Closing(object sender, System.ComponentModel.CancelEventArgs e)
        {
            if (Build.CurrentStatus == BuildViewModel.Status.InProgress)
            {
                Build.Stop();
                e.Cancel = true;
            }
            else if (Build.CurrentStatus == BuildViewModel.Status.Stopping)
            {
                e.Cancel = true;
            }
        }
    }
}
