using System.Windows;
using FilePackager.ViewModels;
using System;
using System.Windows.Input;
using System.Windows.Controls;

namespace FilePackager.Views
{
	/// <summary>
	/// Interaction logic for MainView.xaml
	/// </summary>
	public partial class MainView : Window
	{
		public MainView()
		{
			InitializeComponent();
		}

		private void Window_Closing(object sender, System.ComponentModel.CancelEventArgs e)
		{
			e.Cancel = !(DataContext as MainViewModel).CanCloseProject;

            if (!e.Cancel)
            {
                FilePackager.Properties.Settings.Default.Save();
            }
		}

        private void Window_Closed(object sender, EventArgs e)
        {
            (Application.Current as App).CloseAllPackageLayoutViews();
        }

        private void Window_KeyDown(object sender, System.Windows.Input.KeyEventArgs e)
        {
            if (e.Key == System.Windows.Input.Key.Enter)
            {
                // Pressing "Enter" will:
                //  - force validation to occur
                //  - push the data to the model (ready for save and updating UI)
                if( Keyboard.FocusedElement is TextBox )
                    Base.WPF.Helpers.PushTextBoxChanges();
            }
        }
	}
}
