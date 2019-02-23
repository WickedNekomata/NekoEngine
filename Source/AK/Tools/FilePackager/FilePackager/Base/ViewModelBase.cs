using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Input;
using System.Reflection;
using FilePackager.Undo;
using FilePackager.Models;
using FilePackager.ViewModels;

namespace FilePackager.Base
{
	/// <summary>
	/// Provides common functionality for ViewModel classes
	/// </summary>
	public abstract class ViewModelBase : INotifyPropertyChanged
	{
        private static readonly IEnumerable<string> _noError = new string[0];

        private ModelBase _model;

		public event PropertyChangedEventHandler PropertyChanged;
        public event EventHandler Detached;

		protected void RegisterModel(ModelBase model)
		{
            _model = model;
			_model.PropertyChanged += OnModelPropertyChanged;
		}

        protected virtual void OnModelPropertyChanged(object sender, PropertyChangedEventArgs e)
		{
			// Dispatch the property change
			OnPropertyChanged(e.PropertyName);
		}

		protected void OnPropertyChanged(string propertyName)
		{
			PropertyChangedEventHandler handler = PropertyChanged;
#if DEBUG
            if (GetType().GetProperty(propertyName,
                BindingFlags.Instance | BindingFlags.NonPublic | BindingFlags.Public | BindingFlags.FlattenHierarchy) == null)
                throw new ArgumentException("Property '" + propertyName + "' does not exist on type '" + GetType().Name + "'." );
#endif
            if (handler != null)
			{
				handler(this, new PropertyChangedEventArgs(propertyName));
			}
		}

		public static bool SetValue(object target, string propertyName, object value)
		{
			bool valueChanged = false;

			// Set the value by reflection
			PropertyInfo pi = target.GetType().GetProperty(propertyName);

			if (pi == null)
                throw new ArgumentException("Property '" + propertyName + "' does not exist on type '" + target.GetType().Name + "'.");

			MethodInfo getMethod = pi.GetGetMethod();
			MethodInfo setMethod = pi.GetSetMethod();

			if (getMethod == null || setMethod == null)
                throw new NotSupportedException("set and get must be implemented for the property '" + propertyName + "' on type '" + target.GetType().Name + "'.");

			// If the value are different
			object oldValue = getMethod.Invoke(target, null);
			if (oldValue != value)
			{
				// Save value
				setMethod.Invoke(target, new object[] { value });

				// Store Undo only when target is ModelBase
				ModelBase modelBase = target as ModelBase;
				if (modelBase != null)
				{
					UndoManager.Instance.PushEvent(new PropertyChangeUndoEvent(modelBase, propertyName, oldValue, value));

					// Set Project dirty
                    if (ProjectViewModel.Current != null)
                        ProjectViewModel.Current.IsDirty = true;
				}

				valueChanged = true;
			}

			return valueChanged;
		}

        /// <summary>
        /// Detaches this view model.  Unregister any events.
        /// </summary>
        public virtual void Detach()
        {
            if (_model != null)
                _model.PropertyChanged -= OnModelPropertyChanged;

            if (Detached != null)
            {
                Detached(this, EventArgs.Empty);
            }
        }

        /// <summary>
        /// Gets the errors for this object.
        /// </summary>
        /// <value>The errors.</value>
        public virtual IEnumerable<string> Errors
        {
            get
            {
                return _noError;
            }
        }
	}
}
