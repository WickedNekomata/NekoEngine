using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Controls;
using System.Collections;
using System.Windows.Documents;
using System.Reflection;
using System.Runtime.InteropServices;
using FilePackager.ViewModels;
using FilePackager.Packaging;

namespace FilePackager.Base.WPF
{
	/// <summary>
	/// The drag detector is used to know if the user is moving the mouse
	/// It can't use the WPF functions because it's blocking the UI thread
	/// </summary>
	public class DragDetector
	{
		[DllImport("user32.dll")]
		public static extern bool GetCursorPos(ref System.Drawing.Point pt);

		[DllImport("user32.dll")]
		public static extern bool SetCursorPos(int x, int y);

		[DllImport("user32.dll")]
		private static extern short GetAsyncKeyState(int key);
		private const int VK_LBUTTON = 0x1;
		private const int VK_RBUTTON = 0x2;


		private static bool IsLeftMouseButtonDown()
		{
			int state = GetAsyncKeyState(VK_LBUTTON);
			return (state & 0x8000) != 0;
		}

		/// <summary>
		/// Determines whether the mouse is draggin right now using the Win32 API.
		/// </summary>
		/// <param name="dpo">The dpo.</param>
		/// <returns>
		///     <c>true</c> if the specified dpo is dragging; otherwise, <c>false</c>.
		/// </returns>
		public static bool IsDragging()
		{
			System.Drawing.Point initialPos = new System.Drawing.Point();
			GetCursorPos(ref initialPos);

			System.Drawing.Point currentPos = initialPos;
			while (currentPos == initialPos && IsLeftMouseButtonDown())
			{
				System.Threading.Thread.Sleep(10);
				GetCursorPos(ref currentPos);
			}

			return currentPos != initialPos;
		}
	}

	public class DragDropHelper
	{
		public class DragDropData
		{
            public int IndexInserted { get; set; }
            public object Data { get; set; }
            public bool IsReorder { get; set; }

            public DragDropData()
            {
                IndexInserted = 0;
                IsReorder = false;
            }
		}

		// source and target
		private DataFormat format = DataFormats.GetDataFormat("DragDropItemsControl");
		private Point initialMousePosition;
		private object draggedData;
		private InsertionAdorner insertionAdorner;
		private Window topWindow;
		
		// source
		private ItemsControl sourceItemsControl;
		private FrameworkElement sourceItemContainer;
		
		// target
		private UIElement targetControl;
		private FrameworkElement targetItemContainer;
		private bool hasVerticalOrientation;
		private int insertionIndex;
		private bool isInFirstHalf;
		
		// singleton
		private static DragDropHelper instance;
		private static DragDropHelper Instance 
		{
			get 
			{  
				if(instance == null)
				{
					instance = new DragDropHelper();
				}
				return instance;
			}
		}

		// IsDragSource
		public static bool GetIsDragSource(DependencyObject obj)
		{
			return (bool)obj.GetValue(IsDragSourceProperty);
		}

		public static void SetIsDragSource(DependencyObject obj, bool value)
		{
			obj.SetValue(IsDragSourceProperty, value);
		}

		public static readonly DependencyProperty IsDragSourceProperty =
			DependencyProperty.RegisterAttached("IsDragSource", typeof(bool), typeof(DragDropHelper), new UIPropertyMetadata(false, IsDragSourceChanged));


		// IsDropTarget
		public static bool GetIsDropTarget(DependencyObject obj)
		{
			return (bool)obj.GetValue(IsDropTargetProperty);
		}

		public static void SetIsDropTarget(DependencyObject obj, bool value)
		{
			obj.SetValue(IsDropTargetProperty, value);
		}

		public static readonly DependencyProperty IsDropTargetProperty =
			DependencyProperty.RegisterAttached("IsDropTarget", typeof(bool), typeof(DragDropHelper), new UIPropertyMetadata(false, IsDropTargetChanged));

        //// DragDropTemplate
        //public static DataTemplate GetDragDropTemplate(DependencyObject obj)
        //{
        //    return (DataTemplate)obj.GetValue(DragDropTemplateProperty);
        //}

        //public static void SetDragDropTemplate(DependencyObject obj, DataTemplate value)
        //{
        //    obj.SetValue(DragDropTemplateProperty, value);
        //}

        //public static readonly DependencyProperty DragDropTemplateProperty =
        //    DependencyProperty.RegisterAttached("DragDropTemplate", typeof(DataTemplate), typeof(DragDropHelper), new UIPropertyMetadata(null));

		// DropCommand
		public static ICommand GetDropCommand(DependencyObject obj)
		{
			return (ICommand)obj.GetValue(DropCommandProperty);
		}

		public static void SetDropCommand(DependencyObject obj, ICommand value)
		{
			obj.SetValue(DropCommandProperty, value);
		}

		public static readonly DependencyProperty DropCommandProperty =
			DependencyProperty.RegisterAttached("DropCommand", typeof(ICommand), typeof(DragDropHelper), new UIPropertyMetadata(null));


		private static void IsDragSourceChanged(DependencyObject obj, DependencyPropertyChangedEventArgs e)
		{
			var dragSource = obj as ItemsControl;
			if (dragSource != null)
			{
				if (Object.Equals(e.NewValue, true))
				{
					dragSource.PreviewMouseLeftButtonDown += Instance.DragSource_PreviewMouseLeftButtonDown;
					dragSource.PreviewMouseLeftButtonUp += Instance.DragSource_PreviewMouseLeftButtonUp;
					dragSource.PreviewMouseMove += Instance.DragSource_PreviewMouseMove;
				}
				else
				{
					dragSource.PreviewMouseLeftButtonDown -= Instance.DragSource_PreviewMouseLeftButtonDown;
					dragSource.PreviewMouseLeftButtonUp -= Instance.DragSource_PreviewMouseLeftButtonUp;
					dragSource.PreviewMouseMove -= Instance.DragSource_PreviewMouseMove;
				}
			}
		}

		private static void IsDropTargetChanged(DependencyObject obj, DependencyPropertyChangedEventArgs e)
		{
            var dropTarget = obj as UIElement;
			if (dropTarget != null)
			{
				if (Object.Equals(e.NewValue, true))
				{
					dropTarget.AllowDrop = true;
					dropTarget.PreviewDrop += Instance.DropTarget_PreviewDrop;
					dropTarget.PreviewDragEnter += Instance.DropTarget_PreviewDragEnter;
					dropTarget.PreviewDragOver += Instance.DropTarget_PreviewDragOver;
					dropTarget.PreviewDragLeave += Instance.DropTarget_PreviewDragLeave;
				}
				else
				{
					dropTarget.AllowDrop = false;
					dropTarget.PreviewDrop -= Instance.DropTarget_PreviewDrop;
					dropTarget.PreviewDragEnter -= Instance.DropTarget_PreviewDragEnter;
					dropTarget.PreviewDragOver -= Instance.DropTarget_PreviewDragOver;
					dropTarget.PreviewDragLeave -= Instance.DropTarget_PreviewDragLeave;
				}
			}
		}

		// DragSource

		private void DragSource_PreviewMouseLeftButtonDown(object sender, MouseButtonEventArgs e)
		{
			this.sourceItemsControl = (ItemsControl)sender;
			Visual visual = e.OriginalSource as Visual;

			this.topWindow = (Window)Utilities.FindAncestor(typeof(Window), this.sourceItemsControl);			
			this.initialMousePosition = e.GetPosition(this.topWindow);

			this.sourceItemContainer = Utilities.GetItemContainer(this.sourceItemsControl, visual);

			if (this.sourceItemContainer != null && DragDetector.IsDragging())
			{
				ListBox listBox = this.sourceItemsControl as ListBox;
				if (listBox != null)
				{
                    ListBoxItem lbi = this.sourceItemContainer as ListBoxItem;
                    if (lbi != null && !lbi.IsSelected)
                    {
                        ListBoxHandleMouseDown(lbi);
                    }

                    this.draggedData = listBox.SelectedItems;

					e.Handled = true;
				}
			}
		}

		// Drag = mouse down + move by a certain amount
		private void DragSource_PreviewMouseMove(object sender, MouseEventArgs e)
		{
		    if (this.draggedData != null)
		    {
		        // Only drag when user moved the mouse by a reasonable amount.
		        if (Utilities.IsMovementBigEnough(this.initialMousePosition, e.GetPosition(this.topWindow)))
		        {
		            DataObject data = new DataObject(this.format.Name, this.draggedData);

		            // Adding events to the window to make sure dragged adorner comes up when mouse is not over a drop target.
		            bool previousAllowDrop = this.topWindow.AllowDrop;
		            this.topWindow.AllowDrop = true;
		            this.topWindow.DragEnter += TopWindow_DragEnter;
		            this.topWindow.DragOver += TopWindow_DragOver;
		            this.topWindow.DragLeave += TopWindow_DragLeave;
					
		            DragDrop.DoDragDrop((DependencyObject)sender, data, DragDropEffects.Move);

		            this.topWindow.AllowDrop = previousAllowDrop;
		            this.topWindow.DragEnter -= TopWindow_DragEnter;
		            this.topWindow.DragOver -= TopWindow_DragOver;
		            this.topWindow.DragLeave -= TopWindow_DragLeave;
					
		            this.draggedData = null;
		        }
		    }
		}
			
		private void DragSource_PreviewMouseLeftButtonUp(object sender, MouseButtonEventArgs e)
		{
			this.draggedData = null;
		}

		// DropTarget

		private void DropTarget_PreviewDragEnter(object sender, DragEventArgs e)
		{
			this.targetControl = (UIElement)sender;
			object draggedItem = e.Data.GetData(this.format.Name);

			DecideDropTarget(e);
			if (draggedItem != null)
			{
				// Dragged Adorner is created on the first enter only.
				CreateInsertionAdorner();
			}
			e.Handled = true;
		}

		private void DropTarget_PreviewDragOver(object sender, DragEventArgs e)
		{
			object draggedItem = e.Data.GetData(this.format.Name);

			DecideDropTarget(e);
			if (draggedItem != null)
			{
				// Dragged Adorner is only updated here - it has already been created in DragEnter.
				UpdateInsertionAdornerPosition();
			}
			e.Handled = true;
		}

		private void DropTarget_PreviewDrop(object sender, DragEventArgs e)
		{
			object draggedItem = e.Data.GetData(this.format.Name);

			if (draggedItem != null)
			{
				ICommand command = GetDropCommand(this.targetControl);
				if (command == null)
					throw new InvalidOperationException();

                try
                {
                    command.Execute(GetDragDropData(e));
                }
                catch (Exception exception)
                {
                    MainViewModel.Instance.Application.ShowMessage(
                        string.Format(Properties.Resources.ErrorExecutingCommand, exception.Message),
                        Severity.Error);
                }

				RemoveInsertionAdorner();
			}
			e.Handled = true;
		}

		private DragDropData GetDragDropData(DragEventArgs e)
		{
			object draggedItem = e.Data.GetData(this.format.Name);

			DragDropData data = new DragDropData();
			data.IndexInserted = insertionIndex;
			data.Data = draggedItem;

			if (this.sourceItemsControl == this.targetControl)
			{
                data.IsReorder = true;

                //// This happens when we drag an item to a later position
                //if (data.IndexRemoved != -1 && data.IndexRemoved < this.insertionIndex)
                //{
                //    data.IndexInserted--;
                //}
			}

			return data;
		}

		private void DropTarget_PreviewDragLeave(object sender, DragEventArgs e)
		{
			// Dragged Adorner is only created once on DragEnter + every time we enter the window. 
			// It's only removed once on the DragDrop, and every time we leave the window. (so no need to remove it here)
			object draggedItem = e.Data.GetData(this.format.Name);

			if (draggedItem != null)
			{
				RemoveInsertionAdorner();
			}
			e.Handled = true;
		}

		// If the types of the dragged data and ItemsControl's source are compatible, 
		// there are 3 situations to have into account when deciding the drop target:
		// 1. mouse is over an items container
		// 2. mouse is over the empty part of an ItemsControl, but ItemsControl is not empty
		// 3. mouse is over an empty ItemsControl.
		// The goal of this method is to decide on the values of the following properties: 
		// targetItemContainer, insertionIndex and isInFirstHalf.
		private void DecideDropTarget(DragEventArgs e)
		{
            object draggedItem = e.Data.GetData(this.format.Name);

            if (IsDropDataTypeAllowed(draggedItem, e))
            {
                if (this.targetControl is ItemsControl)
                {
                    ItemsControl itemsControl = this.targetControl as ItemsControl;
                    int targetItemsControlCount = itemsControl.Items.Count;

                    if (targetItemsControlCount > 0)
                    {
                        this.hasVerticalOrientation = Utilities.HasVerticalOrientation(itemsControl.ItemContainerGenerator.ContainerFromIndex(0) as FrameworkElement);
                        this.targetItemContainer = Utilities.GetItemContainer(itemsControl, e.OriginalSource as Visual);

                        if (this.targetItemContainer != null)
                        {
                            Point positionRelativeToItemContainer = e.GetPosition(this.targetItemContainer);
                            this.isInFirstHalf = Utilities.IsInFirstHalf(this.targetItemContainer, positionRelativeToItemContainer, this.hasVerticalOrientation);
                            this.insertionIndex = itemsControl.ItemContainerGenerator.IndexFromContainer(this.targetItemContainer);

                            if (!this.isInFirstHalf)
                            {
                                this.insertionIndex++;
                            }
                        }
                        else
                        {
                            this.targetItemContainer = itemsControl.ItemContainerGenerator.ContainerFromIndex(targetItemsControlCount - 1) as FrameworkElement;
                            this.isInFirstHalf = false;
                            this.insertionIndex = targetItemsControlCount;
                        }
                    }
                    else
                    {
                        this.targetItemContainer = null;
                        this.insertionIndex = 0;
                    }
                }
            }
            else
            {
                this.targetItemContainer = null;
                this.insertionIndex = -1;
                e.Effects = DragDropEffects.None;
            }
		}

		// Can the dragged data be added to the destination collection?
		// It can if destination is bound to IList<allowed type>, IList or not data bound.
		private bool IsDropDataTypeAllowed(object draggedItem, DragEventArgs e)
		{
			bool isDropDataTypeAllowed = false;
			if (draggedItem != null)
			{
				ICommand command = GetDropCommand(this.targetControl);
                if (command != null)
                {
                    try
                    {
                        isDropDataTypeAllowed = command.CanExecute(GetDragDropData(e));
                    }
                    catch
                    { 
#if DEBUG
                        throw new InvalidOperationException("Could not execute the CanExecute");
#endif
                    }
                }
			}
			else
			{
				isDropDataTypeAllowed = false;			
			}
			return isDropDataTypeAllowed;
		}

		// Window

		private void TopWindow_DragEnter(object sender, DragEventArgs e)
		{
			//ShowDraggedAdorner(e.GetPosition(this.topWindow));
			e.Effects = DragDropEffects.None;
			e.Handled = true;
		}

		private void TopWindow_DragOver(object sender, DragEventArgs e)
		{
			//ShowDraggedAdorner(e.GetPosition(this.topWindow));
			e.Effects = DragDropEffects.None;
			e.Handled = true;
		}

		private void TopWindow_DragLeave(object sender, DragEventArgs e)
		{
			e.Handled = true;
		}

		// Adorners

		private void CreateInsertionAdorner()
		{
			if (this.targetItemContainer != null)
			{
                // Here, I need to get adorner layer from targetItemContainer and not targetItemsControl. 
				// This way I get the AdornerLayer within ScrollContentPresenter, and not the one under AdornerDecorator (Snoop is awesome).
                // If I used targetItemsControl, the adorner would hang out of ItemsControl when there's a horizontal scroll bar.
				var adornerLayer = AdornerLayer.GetAdornerLayer(this.targetItemContainer);
				this.insertionAdorner = new InsertionAdorner(this.hasVerticalOrientation, this.isInFirstHalf, this.targetItemContainer, adornerLayer);
			}
		}

		private void UpdateInsertionAdornerPosition()
		{
			if (this.insertionAdorner != null)
			{
				this.insertionAdorner.IsInFirstHalf = this.isInFirstHalf;
				this.insertionAdorner.InvalidateVisual();
			}
		}

		private void RemoveInsertionAdorner()
		{
			if (this.insertionAdorner != null)
			{
				this.insertionAdorner.Detach();
				this.insertionAdorner = null;
			}
		}

        public static void ListBoxHandleMouseDown(ListBoxItem lbi)
        {
            // The default implementation of Thumb.OnMouseLeftButtonDown set the e.Handled to true
            // and eats the mouse click so the ListBoxItem does not get a chance to update its 
            // selection.
            // We call the internal function HandleMouseButtonDown to tell the ListBoxItem to handle
            // the click and update the selection.
            MethodInfo method = typeof(ListBoxItem).GetMethod(
                "HandleMouseButtonDown", BindingFlags.NonPublic | BindingFlags.Instance);

            if (method != null && lbi != null)
            {
                method.Invoke(lbi, new object[] { MouseButton.Left });
            }
        }
	}
}
