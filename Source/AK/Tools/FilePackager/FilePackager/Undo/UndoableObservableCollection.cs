using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Collections.ObjectModel;
using System.Collections.Specialized;
using FilePackager.Base;
using System.Windows;
using FilePackager.ViewModels;

namespace FilePackager.Undo
{
	public class UndoableObservableCollection<T,SourceT> : ObservableCollection<T>
        where T : ViewModelBase 
		where SourceT: class
	{
		private ConstructorDelegate _ctor;
		private ObservableCollection<SourceT> _source;

		public delegate T ConstructorDelegate(SourceT source);

		public UndoableObservableCollection( ObservableCollection<SourceT> source, ConstructorDelegate ctor )
		{
			_ctor = ctor;
			_source = source;
			
			// Initialize from source
			this.AddRange(source.Select(s => _ctor(s)));

            _source.CollectionChanged += source_CollectionChanged;
		}

		void source_CollectionChanged(object sender, NotifyCollectionChangedEventArgs e)
		{
			switch (e.Action)
			{
				case NotifyCollectionChangedAction.Add:
					{
						int index = e.NewStartingIndex;
						foreach (SourceT item in e.NewItems)
						{
							Insert(index, _ctor(item));

							UndoManager.Instance.PushEvent(new CollectionChangeUndoEvent(_source, e.Action, item, index));

                            ProjectViewModel.Current.IsDirty = true;

							++index;
						}
					}
					break;
				case NotifyCollectionChangedAction.Remove:
					{
						int index = e.OldStartingIndex;
						foreach (SourceT item in e.OldItems)
						{
                            this[index].Detach();
							RemoveItem(index);

							UndoManager.Instance.PushEvent(new CollectionChangeUndoEvent(_source, e.Action, item, index));

                            ProjectViewModel.Current.IsDirty = true;
						}
					}
					break;

				case NotifyCollectionChangedAction.Move:
				case NotifyCollectionChangedAction.Replace:
				case NotifyCollectionChangedAction.Reset:
				default:
					throw new NotSupportedException();
			}
		}

        public void Detach()
        {
            _source.CollectionChanged -= source_CollectionChanged;

            foreach (T item in this)
            {
                item.Detach();
            }
        }
	}
}
