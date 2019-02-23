using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.ComponentModel;
using System.Timers;

namespace FilePackager.Base.WPF
{
    /// <summary>
    /// Interaction logic for EditLabel.xaml
    /// </summary>
    public partial class EditLabel : UserControl
    {
        private string _textBeforeEdit;
        private Timer _timer = new Timer((double)System.Windows.Forms.SystemInformation.DoubleClickTime);

        // Text
        #region Text Property
        /// <summary>
        /// Gets or sets the text.
        /// </summary>
        /// <value>The text.</value>
        public string Text
        {
            get { return (string)GetValue( TextProperty ); }
            set { SetValue( TextProperty, value ); }
        }

        public static readonly DependencyProperty TextProperty =
            DependencyProperty.Register("Text", typeof(string), typeof(EditLabel), 
            new FrameworkPropertyMetadata(String.Empty, 
                FrameworkPropertyMetadataOptions.BindsTwoWayByDefault | FrameworkPropertyMetadataOptions.AffectsRender | FrameworkPropertyMetadataOptions.AffectsArrange));

        private void Renamable_RenameRequested(object sender, EventArgs e)
        {
            IsEditing = true;
        }

        #endregion

        // IsEditing
        #region IsEditing
        /// <summary>
        /// Gets or sets a value indicating whether this instance is in edit mode.
        /// </summary>
        /// <value>
        ///     <c>true</c> if this instance is editing; otherwise, <c>false</c>.
        /// </value>
        public bool IsEditing
        {
            get { return (bool)GetValue( IsEditingProperty ); }
            set { SetValue( IsEditingProperty, value ); }
        }

        public static readonly DependencyProperty IsEditingProperty =
            DependencyProperty.Register( "IsEditing", typeof( bool ), typeof( EditLabel ), new UIPropertyMetadata( false, OnIsEditingPropertyChanged ) );

        public static void OnIsEditingPropertyChanged( DependencyObject d, DependencyPropertyChangedEventArgs e )
        {
            EditLabel pThis = (EditLabel)d;

            pThis.Edit( (bool)e.NewValue );
        }

        private void Edit( bool bEdit )
        {
            if( bEdit )
            {
                // Fall in edit mode
                _edit.Visibility = Visibility.Visible;
                _label.Visibility = Visibility.Collapsed;

                _textBeforeEdit = Text;

                ListBox listBox = Helpers.FindParent<ListBox>(this);
                if (listBox != null)
                {
                    listBox.SelectedItem = DataContext;
                }

                _edit.Focus();
                _edit.Select(0, _textBeforeEdit.Length - System.IO.Path.GetExtension(_textBeforeEdit).Length);
            }
            else
            {
                _edit.Visibility = Visibility.Collapsed;
                _label.Visibility = Visibility.Visible;
            }
        } 
        #endregion

        private void PushChanges()
        {
            // Remove leading/trailing spaces
            _edit.Text = _edit.Text.Trim();

            BindingExpression exp = _edit.GetBindingExpression(TextBox.TextProperty);
            if (exp != null)
            {
                exp.UpdateSource();
            }
        }

        public bool HasError
        {
            get
            {
                bool hasError = false;
                BindingExpression exp = GetBindingExpression(TextProperty);
                if (exp != null)
                {
                    hasError = exp.HasError;
                }

                return hasError;
            }
        }
        
        public EditLabel()
        {
            InitializeComponent();
        }

        private void EditLabel_Loaded(object sender, RoutedEventArgs e)
        {
            _timer.Elapsed += Timer_Elapsed;

            IRenamable renamable = DataContext as IRenamable;
            if (renamable != null)
            {
                renamable.RenameRequested += Renamable_RenameRequested;
            }
        }

        private void EditLabel_Unloaded(object sender, RoutedEventArgs e)
        {
            IRenamable renamable = DataContext as IRenamable;
            if (renamable != null)
            {
                renamable.RenameRequested -= Renamable_RenameRequested;
            }

            _timer.Elapsed -= Timer_Elapsed;
        }

        private void _edit_PreviewKeyDown( object sender, KeyEventArgs e )
        {
            if( e.Key == Key.Escape )
            {
                _edit.Text = _textBeforeEdit;
                IsEditing = false;
            }
            else if( e.Key == Key.Enter )
            {
                PushChanges();

                if (!HasError)
                {
                    ListBoxItem lbi = Helpers.FindParent<ListBoxItem>(this);

                    if (lbi != null)
                    {
                        lbi.Focus();
                    }
                }

                e.Handled = true;
            }
        }

        private void _edit_LostFocus( object sender, RoutedEventArgs e )
        {
            PushChanges();

            if (!HasError)
                IsEditing = false;
        }

        private void _label_MouseDown(object sender, MouseButtonEventArgs e)
        {
            ListBoxItem lbi = Helpers.FindParent<ListBoxItem>(this);

            if (lbi != null && lbi.IsSelected)
            {
                if (e.ClickCount == 1)
                {
                    _timer.Enabled = true;
                }
                else
                {
                    // Double-click: Cancel editing
                    _timer.Enabled = false;
                } 
                
                e.Handled = true;
            } 
        }

        void Timer_Elapsed(object sender, ElapsedEventArgs e)
        {
            _timer.Enabled = false;

            // Get out of the timer thread
            Dispatcher.BeginInvoke(System.Windows.Threading.DispatcherPriority.Normal,
                (Action)delegate
                {
                    IsEditing = true;
                });
        }
    }
}
