using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Input;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Media;

namespace FilePackager.Base.WPF
{
	static public class Helpers
	{
		public static void PushTextBoxChanges()
		{
			// If the current field with focus is a TextBox 
			// Force the binding to update now because no kill focus will be trigged
			IInputElement element = Keyboard.FocusedElement;
            TextBox textBox = (element as TextBox);
            if (textBox != null)
			{
				// If we call this function when pressing ENTER, make sure we don't push the changes
				// if we are inside a multiline textbox.
				if (!Keyboard.IsKeyDown(Key.Return) ||
					!textBox.AcceptsReturn)
				{
                    BindingExpression exp =
                        textBox.GetBindingExpression(TextBox.TextProperty);

                    if (exp != null &&
                        exp.ParentBinding != null &&
                        (exp.ParentBinding.UpdateSourceTrigger == UpdateSourceTrigger.LostFocus ||
                        exp.ParentBinding.UpdateSourceTrigger == UpdateSourceTrigger.Default ||
                        exp.ParentBinding.UpdateSourceTrigger == UpdateSourceTrigger.Explicit))
                    {
                        exp.UpdateSource();

                        if (exp.HasError)
                        {
                            // Remain in the text box when there is an error
                            textBox.SelectAll();
                        }
                        else
                        {
                            // Move to a parent that can take focus
                            FrameworkElement parent = (FrameworkElement)textBox.Parent;
                            while (parent != null && parent is IInputElement && !((IInputElement)parent).Focusable)
                            {
                                parent = (FrameworkElement)parent.Parent;
                            }

                            DependencyObject scope = FocusManager.GetFocusScope(textBox);
                            FocusManager.SetFocusedElement(scope, parent as IInputElement);
                        }
                    }
				}
			}
		}

        /// <summary>
        /// Recursively finds a WPF object of a specified type in the visual parents.
        /// </summary>
        /// <typeparam name="T">Type of parent to look at.</typeparam>
        /// <param name="from">Where to start looking at.</param>
        /// <returns>The first parent found of the requested type.</returns>
        public static T FindParent<T>(DependencyObject from)
            where T : class
        {
            T result = null;

            DependencyObject parent = VisualTreeHelper.GetParent(from);

            if (parent is T)
                result = parent as T;
            else if (parent != null)
                result = FindParent<T>(parent);

            return result;
        }
	}
}
