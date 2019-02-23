using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Reflection;
using FilePackager.ViewModels;
using FilePackager.Models;
using FilePackager.Base;

namespace FilePackager.Undo
{
	class PropertyChangeUndoEvent : IUndoEvent
	{
		private ModelBase _target;
		private string _propertyName;
		private object _oldValue;
		private object _newValue;

		public PropertyChangeUndoEvent(ModelBase target, string propertyName, object oldValue, object newValue)
		{
			_target = target;
			_propertyName = propertyName;
			_oldValue = oldValue;
			_newValue = newValue;
		}

		public void Undo()
		{
			SetValue(_oldValue);
		}

		public void Redo()
		{
			SetValue(_newValue);
		}
		
		private void SetValue(object value)
		{
			ViewModelBase.SetValue(_target, _propertyName, value);
		}
	}
}
