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
    /// Interaction logic for PackageLayoutView.xaml
    /// </summary>
    public partial class PackageLayoutView : Window
    {
        public PackageLayoutView()
        {
            InitializeComponent();
        }
        
        protected override void OnPropertyChanged(DependencyPropertyChangedEventArgs e)
        {
            base.OnPropertyChanged(e);

            if (e.Property == DataContextProperty)
            {
                if (e.OldValue != null)
                {
                    PackageViewModel viewModel = (e.OldValue as PackageViewModel);
                    viewModel.Detached -= Package_Detached;
                    viewModel.PreUnlayoutItemsChanged -= Package_PreUnlayoutItemsChanged;
                }

                if (e.NewValue != null)
                {
                    PackageViewModel viewModel = (e.NewValue as PackageViewModel);
                    viewModel.Detached += Package_Detached;
                    viewModel.PreUnlayoutItemsChanged += Package_PreUnlayoutItemsChanged;
                }
            }
        }

        void Package_PreUnlayoutItemsChanged(object sender, EventArgs e)
        {
            // Clear the selection for performance issues (WG-15843)
            _unlayoutItems.SelectedItems.Clear();
        }

        private void Package_Detached(object sender, EventArgs e)
        {
            Close();
        }
    }
}
