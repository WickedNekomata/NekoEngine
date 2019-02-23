using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace FilePackager.Undo
{
	internal class UndoManager
	{
		private static UndoManager _instance = null;
		private Stack<IUndoEvent> _undoEvents = new Stack<IUndoEvent>();
		private Stack<IUndoEvent> _redoEvents = new Stack<IUndoEvent>();

        private Stack<EventGroup> _activeGroups = new Stack<EventGroup>();

        private IUndoEvent _currentUndoEvent = null;

        public event EventHandler IsBusyOrGroupFinished;

        /// <summary>
        /// Group many events together
        /// </summary>
        private class EventGroup : IUndoEvent
        {
            private List<IUndoEvent> _events = new List<IUndoEvent>();

            public void Undo()
            {
                foreach (IUndoEvent undoEvent in _events.Reverse<IUndoEvent>())
                {
                    undoEvent.Undo();
                }
            }

            public void Redo()
            {
                foreach (IUndoEvent undoEvent in _events)
                {
                    undoEvent.Redo();
                }
            }

            public void PushEvent(IUndoEvent undoEvent)
            {
                _events.Add(undoEvent);
            }
        }

        /// <summary>
        /// Use this to open and close a undo group.  Use the using statement.
        /// </summary>
        public class EventGroupScope : IDisposable
        {
            public EventGroupScope()
            {
                UndoManager.Instance.PushGroup();
            }
            public void Dispose()
            {
                UndoManager.Instance.PopGroup();
            }
        }

		private UndoManager()
		{
		}

		internal static UndoManager Instance
		{
			get 
			{
				if (_instance == null)
				{
					_instance = new UndoManager();
				}
				return _instance; 
			}
		}

		internal void Clear()
		{
			_undoEvents.Clear();
		}

		internal void Undo()
		{
			UndoRedo(_undoEvents, _redoEvents);
		}

		internal void Redo()
		{
			UndoRedo(_redoEvents, _undoEvents);
		}

		private void UndoRedo( Stack<IUndoEvent> from, Stack<IUndoEvent> to)
		{
			if (IsBusy)
				throw new InvalidOperationException();

            using (new FilePackager.Base.WPF.AutoCursor(System.Windows.Input.Cursors.Wait))
            {
                try
                {
                    if (!from.Any())
                        throw new InvalidOperationException();

                    _currentUndoEvent = from.Pop();

                    // Do it
                    if (from == _undoEvents)
                        _currentUndoEvent.Undo();
                    else
                        _currentUndoEvent.Redo();

                    to.Push(_currentUndoEvent);
                }
                finally
                {
                    _currentUndoEvent = null;

                    // Notify
                    if (IsBusyOrGroupFinished != null)
                    {
                        IsBusyOrGroupFinished(this, EventArgs.Empty);
                    }
                }
            }
		}

		internal bool CanUndo()
		{
			return _undoEvents.Any();
		}

		internal bool CanRedo()
		{
			return _redoEvents.Any();
		}

		internal void PushEvent(IUndoEvent undoEvent)
		{
			if (!IsBusy)
			{
                if (_activeGroups.Any())
                {
                    _activeGroups.Peek().PushEvent(undoEvent);
                }
                else
                {
                    _undoEvents.Push(undoEvent);
                    _redoEvents.Clear();
                }
			}
		}

		internal bool IsBusy
		{
			get
			{
				return _currentUndoEvent != null;
			}
		}

        internal void PushGroup()
        {
            _activeGroups.Push(new EventGroup());
        }

        internal void PopGroup()
        {
            PushEvent(_activeGroups.Pop());

            if (!_activeGroups.Any() && IsBusyOrGroupFinished != null)
            {
                IsBusyOrGroupFinished(this, EventArgs.Empty);
            }
        }

        internal bool HasActiveGroup
        {
            get
            {
                return _activeGroups.Any();
            }
        }

        public void RegisterOnceToIsBusyOrGroupFinished(EventHandler handler)
        {
            if (IsBusyOrGroupFinished == null ||
                !IsBusyOrGroupFinished.GetInvocationList().Contains(handler))
            {
                IsBusyOrGroupFinished += handler;
            }
        }
    }
}
