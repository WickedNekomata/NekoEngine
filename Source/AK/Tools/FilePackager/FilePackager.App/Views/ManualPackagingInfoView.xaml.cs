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
using System.Windows.Navigation;
using System.Windows.Shapes;
using FilePackager.ViewModels;

namespace FilePackager.Views
{
	/// <summary>
	/// Interaction logic for ManualPackagingInfoView.xaml
	/// </summary>
	public partial class ManualPackagingInfoView : UserControl
	{
		public ManualPackagingInfoView()
		{
			InitializeComponent();
		}

        void ListBoxItem_MouseDoubleClick(object sender, MouseButtonEventArgs e)
        {
            ManualPackagingInfoViewModel viewModel = (DataContext as ManualPackagingInfoViewModel);
            if (viewModel != null && viewModel.CurrentPackage != null)
            {
                viewModel.CurrentPackage.EditLayoutCommand.Execute(null);
                e.Handled = true;
            }
        }
	}
}
