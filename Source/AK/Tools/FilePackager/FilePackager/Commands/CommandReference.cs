using System;
using System.Windows;
using System.Windows.Input;

namespace FilePackager.Commands
{
	/// <summary>
	/// This class facilitates associating a key binding in XAML markup to a command
	/// defined in a View Model by exposing a Command dependency property.
	/// The class derives from Freezable to work around a limitation in WPF when data-binding from XAML.
	/// </summary>
	public class CommandReference : Freezable, ICommand
	{
		public CommandReference()
		{
			// Blank
		}

		public static readonly DependencyProperty CommandProperty = DependencyProperty.Register("Command", typeof(ICommand), typeof(CommandReference), new PropertyMetadata(new PropertyChangedCallback(OnCommandChanged)));
        public static readonly DependencyProperty CommandParameterProperty = DependencyProperty.Register("CommandParameter", typeof(object), typeof(CommandReference), new UIPropertyMetadata(null));

		public ICommand Command
		{
			get { return (ICommand)GetValue(CommandProperty); }
			set { SetValue(CommandProperty, value); }
		}

        public object CommandParameter
        {
            get { return (object)GetValue(CommandParameterProperty); }
            set { SetValue(CommandParameterProperty, value); }
        }

		#region ICommand Members

		public bool CanExecute(object parameter)
		{
			if (Command != null)
                return Command.CanExecute(CommandParameter);
			return false;
		}

		public void Execute(object parameter)
		{
            Command.Execute(CommandParameter);
		}

		public event EventHandler CanExecuteChanged;

		private static void OnCommandChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
		{
			CommandReference commandReference = d as CommandReference;
			ICommand oldCommand = e.OldValue as ICommand;
			ICommand newCommand = e.NewValue as ICommand;

			if (oldCommand != null)
			{
				oldCommand.CanExecuteChanged -= commandReference.CanExecuteChanged;
			}
			if (newCommand != null)
			{
				newCommand.CanExecuteChanged += commandReference.CanExecuteChanged;
			}
		}

		#endregion

		#region Freezable

		protected override Freezable CreateInstanceCore()
		{
			throw new NotImplementedException();
		}

		#endregion
	}
}
