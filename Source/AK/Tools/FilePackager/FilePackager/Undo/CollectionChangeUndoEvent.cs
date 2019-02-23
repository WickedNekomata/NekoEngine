using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Collections;
using System.Collections.Specialized;

namespace FilePackager.Undo
{
	class CollectionChangeUndoEvent : IUndoEvent
	{
		private IList _target;
		private NotifyCollectionChangedAction _action;
		private object _item;
		private int _index;

		public CollectionChangeUndoEvent( IList target, NotifyCollectionChangedAction action, object item, int index )
		{
			if (!(action == NotifyCollectionChangedAction.Add || action == NotifyCollectionChangedAction.Remove))
				throw new NotSupportedException();

			_target = target;
			_action = action;
			_item = item;
			_index = index;
		}

		public void Undo()
		{
			if (_action == NotifyCollectionChangedAction.Add)
				Remove();
			else
				Insert();
		}

		public void Redo()
		{
			if (_action == NotifyCollectionChangedAction.Add)
				Insert();
			else
				Remove();
		}

		private void Insert()
		{
			_target.Insert(_index, _item);
		}

		private void Remove()
		{
			_target.RemoveAt(_index);
		}
	}
}
