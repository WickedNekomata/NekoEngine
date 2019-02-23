using System;
using System.Collections.Generic;
using System.Windows;
using System.Windows.Input;
using FilePackager.ViewModels;
using FilePackager.Packaging;

namespace FilePackager.Commands
{
	/// <summary>
	///     This class allows delegating the commanding logic to methods passed as parameters,
	///     and enables a View to bind commands to objects that are not part of the element tree.
	/// </summary>
	public class DelegateCommand : ICommand
	{
		public delegate void FuncExecute(object parameter);
		public delegate bool FuncCanExecute(object parameter);

		#region Constructors


		/// <summary>
		///     Constructor
		/// </summary>
		public DelegateCommand(FuncExecute executeMethod, FuncCanExecute canExecuteMethod, bool isAutomaticRequeryDisabled, string name)
		{
			if (executeMethod == null)
			{
				throw new ArgumentNullException("executeMethod");
			}

            _name = name;
			_executeMethod = executeMethod;
			_canExecuteMethod = canExecuteMethod;

			_isAutomaticRequeryDisabled = isAutomaticRequeryDisabled;
		}
		#endregion

		#region Public Methods

		/// <summary>
		///     Method to determine if the command can be executed
		/// </summary>
		public bool CanExecute(object parameter)
		{
            try
            {
                if (_canExecuteMethod != null)
                {
                    return _canExecuteMethod(parameter);
                }
            }
            catch
            {
                // Don't show anything
#if DEBUG
                throw new InvalidOperationException("Could not execute the CanExecute");
#else
                return false;
#endif
            }

			return true;
		}

		/// <summary>
		///     Execution of the command
		/// </summary>
		public bool Execute(object parameter)
		{
            bool succeeded = true;
            try
            {
                if (_executeMethod != null)
                {
                    _executeMethod(parameter);
                }
            }
            catch (Exception e)
            {
                MainViewModel.Instance.Application.ShowMessage(
                    string.Format(Properties.Resources.ErrorExecutingCommand, Name, e.Message),
                    Severity.Error);
                
                succeeded = false;
            }

            return succeeded;
		}

		/// <summary>
		///     Property to enable or disable CommandManager's automatic requery on this command
		/// </summary>
		public bool IsAutomaticRequeryDisabled
		{
			get
			{
				return _isAutomaticRequeryDisabled;
			}
			set
			{
				if (_isAutomaticRequeryDisabled != value)
				{
					if (value)
					{
						CommandManagerHelper.RemoveHandlersFromRequerySuggested(_canExecuteChangedHandlers);
					}
					else
					{
						CommandManagerHelper.AddHandlersToRequerySuggested(_canExecuteChangedHandlers);
					}
					_isAutomaticRequeryDisabled = value;
				}
			}
		}

        public string Name
        {
            get
            {
                if (!string.IsNullOrEmpty(_name))
                {
                    return _name;
                }

                // If no name was provided, use the method name
                return _executeMethod.Method.Name;
            }
        }

		/// <summary>
		///     Raises the CanExecuteChaged event
		/// </summary>
		public void RaiseCanExecuteChanged()
		{
			OnCanExecuteChanged();
		}

		/// <summary>
		///     Protected virtual method to raise CanExecuteChanged event
		/// </summary>
		protected virtual void OnCanExecuteChanged()
		{
			CommandManagerHelper.CallWeakReferenceHandlers(_canExecuteChangedHandlers);
		}

		#endregion

		#region ICommand Members

		/// <summary>
		///     ICommand.CanExecuteChanged implementation
		/// </summary>
		public event EventHandler CanExecuteChanged
		{
			add
			{
				if (!_isAutomaticRequeryDisabled)
				{
					CommandManager.RequerySuggested += value;
				}
				CommandManagerHelper.AddWeakReferenceHandler(ref _canExecuteChangedHandlers, value, 2);
			}
			remove
			{
				if (!_isAutomaticRequeryDisabled)
				{
					CommandManager.RequerySuggested -= value;
				}
				CommandManagerHelper.RemoveWeakReferenceHandler(_canExecuteChangedHandlers, value);
			}
		}

		bool ICommand.CanExecute(object parameter)
		{
			return CanExecute(parameter);
		}

		void ICommand.Execute(object parameter)
		{
			Execute(parameter);
		}

		#endregion

		#region Data

		private readonly FuncExecute _executeMethod = null;
		private readonly FuncCanExecute _canExecuteMethod = null;
		private bool _isAutomaticRequeryDisabled = false;
		private List<WeakReference> _canExecuteChangedHandlers;
        private string _name;

		#endregion
	}

	/// <summary>
	///     This class allows delegating the commanding logic to methods passed as parameters,
	///     and enables a View to bind commands to objects that are not part of the element tree.
	/// </summary>
	/// <typeparam name="T">Type of the parameter passed to the delegates</typeparam>
	public class DelegateCommand<T> : ICommand
	{
		#region Constructors

		/// <summary>
		///     Constructor
		/// </summary>
		public DelegateCommand(Action<T> executeMethod)
			: this(executeMethod, null, false)
		{
		}

		/// <summary>
		///     Constructor
		/// </summary>
		public DelegateCommand(Action<T> executeMethod, Func<T, bool> canExecuteMethod)
			: this(executeMethod, canExecuteMethod, false)
		{
		}

		/// <summary>
		///     Constructor
		/// </summary>
		public DelegateCommand(Action<T> executeMethod, Func<T, bool> canExecuteMethod, bool isAutomaticRequeryDisabled)
		{
			if (executeMethod == null)
			{
				throw new ArgumentNullException("executeMethod");
			}

			_executeMethod = executeMethod;
			_canExecuteMethod = canExecuteMethod;
			_isAutomaticRequeryDisabled = isAutomaticRequeryDisabled;
		}

		#endregion

		#region Public Methods

		/// <summary>
		///     Method to determine if the command can be executed
		/// </summary>
		public bool CanExecute(T parameter)
		{
			if (_canExecuteMethod != null)
			{
				return _canExecuteMethod(parameter);
			}
			return true;
		}

		/// <summary>
		///     Execution of the command
		/// </summary>
		public void Execute(T parameter)
		{
			if (_executeMethod != null)
			{
				_executeMethod(parameter);
			}
		}

		/// <summary>
		///     Raises the CanExecuteChaged event
		/// </summary>
		public void RaiseCanExecuteChanged()
		{
			OnCanExecuteChanged();
		}

		/// <summary>
		///     Protected virtual method to raise CanExecuteChanged event
		/// </summary>
		protected virtual void OnCanExecuteChanged()
		{
			CommandManagerHelper.CallWeakReferenceHandlers(_canExecuteChangedHandlers);
		}

		/// <summary>
		///     Property to enable or disable CommandManager's automatic requery on this command
		/// </summary>
		public bool IsAutomaticRequeryDisabled
		{
			get
			{
				return _isAutomaticRequeryDisabled;
			}
			set
			{
				if (_isAutomaticRequeryDisabled != value)
				{
					if (value)
					{
						CommandManagerHelper.RemoveHandlersFromRequerySuggested(_canExecuteChangedHandlers);
					}
					else
					{
						CommandManagerHelper.AddHandlersToRequerySuggested(_canExecuteChangedHandlers);
					}
					_isAutomaticRequeryDisabled = value;
				}
			}
		}

		#endregion

		#region ICommand Members

		/// <summary>
		///     ICommand.CanExecuteChanged implementation
		/// </summary>
		public event EventHandler CanExecuteChanged
		{
			add
			{
				if (!_isAutomaticRequeryDisabled)
				{
					CommandManager.RequerySuggested += value;
				}
				CommandManagerHelper.AddWeakReferenceHandler(ref _canExecuteChangedHandlers, value, 2);
			}
			remove
			{
				if (!_isAutomaticRequeryDisabled)
				{
					CommandManager.RequerySuggested -= value;
				}
				CommandManagerHelper.RemoveWeakReferenceHandler(_canExecuteChangedHandlers, value);
			}
		}

		bool ICommand.CanExecute(object parameter)
		{
			// if T is of value type and the parameter is not
			// set yet, then return false if CanExecute delegate
			// exists, else return true
			if (parameter == null &&
				typeof(T).IsValueType)
			{
				return (_canExecuteMethod == null);
			}
			return CanExecute((T)parameter);
		}

		void ICommand.Execute(object parameter)
		{
			Execute((T)parameter);
		}

		#endregion

		#region Data

		private readonly Action<T> _executeMethod = null;
		private readonly Func<T, bool> _canExecuteMethod = null;
		private bool _isAutomaticRequeryDisabled = false;
		private List<WeakReference> _canExecuteChangedHandlers;

		#endregion
	}

	/// <summary>
	///     This class contains methods for the CommandManager that help avoid memory leaks by
	///     using weak references.
	/// </summary>
	internal class CommandManagerHelper
	{
		internal static void CallWeakReferenceHandlers(List<WeakReference> handlers)
		{
			if (handlers != null)
			{
				// Take a snapshot of the handlers before we call out to them since the handlers
				// could cause the array to me modified while we are reading it.

				EventHandler[] callees = new EventHandler[handlers.Count];
				int count = 0;

				for (int i = handlers.Count - 1; i >= 0; i--)
				{
					WeakReference reference = handlers[i];
					EventHandler handler = reference.Target as EventHandler;
					if (handler == null)
					{
						// Clean up old handlers that have been collected
						handlers.RemoveAt(i);
					}
					else
					{
						callees[count] = handler;
						count++;
					}
				}

				// Call the handlers that we snapshotted
				for (int i = 0; i < count; i++)
				{
					EventHandler handler = callees[i];
					handler(null, EventArgs.Empty);
				}
			}
		}

		internal static void AddHandlersToRequerySuggested(List<WeakReference> handlers)
		{
			if (handlers != null)
			{
				foreach (WeakReference handlerRef in handlers)
				{
					EventHandler handler = handlerRef.Target as EventHandler;
					if (handler != null)
					{
						CommandManager.RequerySuggested += handler;
					}
				}
			}
		}

		internal static void RemoveHandlersFromRequerySuggested(List<WeakReference> handlers)
		{
			if (handlers != null)
			{
				foreach (WeakReference handlerRef in handlers)
				{
					EventHandler handler = handlerRef.Target as EventHandler;
					if (handler != null)
					{
						CommandManager.RequerySuggested -= handler;
					}
				}
			}
		}

		internal static void AddWeakReferenceHandler(ref List<WeakReference> handlers, EventHandler handler)
		{
			AddWeakReferenceHandler(ref handlers, handler, -1);
		}

		internal static void AddWeakReferenceHandler(ref List<WeakReference> handlers, EventHandler handler, int defaultListSize)
		{
			if (handlers == null)
			{
				handlers = (defaultListSize > 0 ? new List<WeakReference>(defaultListSize) : new List<WeakReference>());
			}

			handlers.Add(new WeakReference(handler));
		}

		internal static void RemoveWeakReferenceHandler(List<WeakReference> handlers, EventHandler handler)
		{
			if (handlers != null)
			{
				for (int i = handlers.Count - 1; i >= 0; i--)
				{
					WeakReference reference = handlers[i];
					EventHandler existingHandler = reference.Target as EventHandler;
					if ((existingHandler == null) || (existingHandler == handler))
					{
						// Clean up old handlers that have been collected
						// in addition to the handler that is to be removed.
						handlers.RemoveAt(i);
					}
				}
			}
		}

        internal static ICommand CreateOnceCommand(ref DelegateCommand command, Action executeMethod, Func<bool> canExecuteMethod)
        {
            return CreateOnceCommand(ref command, executeMethod, canExecuteMethod, null);
        }

		internal static ICommand CreateOnceCommand(ref DelegateCommand command, Action executeMethod, Func<bool> canExecuteMethod, string name)
		{
			if (command == null)
			{
				if (executeMethod == null)
				{
					throw new ArgumentNullException("executeMethod");
				}

                if (string.IsNullOrEmpty(name))
                {
                    name = executeMethod.Method.Name;
                }

				DelegateCommand.FuncExecute execute =
					delegate
					{
						executeMethod();
					};
				DelegateCommand.FuncCanExecute canExecute = null;
				if(canExecuteMethod != null )
				{
					canExecute = delegate(object parameter)
					{
						return canExecuteMethod();
					};
				}
				command = new DelegateCommand(execute, canExecute,false, name);
			}
			return command;
		}

        internal static ICommand CreateOnceCommand(ref DelegateCommand command, DelegateCommand.FuncExecute execute, DelegateCommand.FuncCanExecute canExecute)
        {
            return CreateOnceCommand(ref command, execute, canExecute, null);
        }

		internal static ICommand CreateOnceCommand(ref DelegateCommand command, DelegateCommand.FuncExecute execute, DelegateCommand.FuncCanExecute canExecute, string name)
		{
			if (command == null)
			{
				command = new DelegateCommand(execute, canExecute,false, name);
			}
			return command;
		}
	}
}