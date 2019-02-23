using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.ComponentModel;
using System.Configuration;
using System.Windows;

namespace FilePackager.Base.WPF
{
        /// <summary>
        /// Persists a Window's Size, Location and WindowState to UserScopeSettings 
        /// </summary>
    public class WindowPosition
    {
        #region WindowApplicationSettings Helper Class
        public class WindowApplicationSettings : ApplicationSettingsBase
        {
            private WindowPosition windowPosition;

            public WindowApplicationSettings(WindowPosition windowPosition)
                : base(windowPosition.window.GetType().FullName)
            {
                this.windowPosition = windowPosition;
            }

            [UserScopedSetting]
            public Rect Location
            {
                get
                {
                    if (this["Location"] != null)
                    {
                        return ((Rect)this["Location"]);
                    }
                    return Rect.Empty;
                }
                set
                {
                    this["Location"] = value;
                }
            }

            [UserScopedSetting]
            public WindowState WindowState
            {
                get
                {
                    if (this["WindowState"] != null)
                    {
                        return (WindowState)this["WindowState"];
                    }
                    return WindowState.Normal;
                }
                set
                {
                    this["WindowState"] = value;
                }
            }

        }
        #endregion

        #region Constructor
        private Window window = null;

        public WindowPosition(Window window)
        {
            this.window = window;
        }

        #endregion

        #region Attached "Save" Property Implementation
        /// <summary>
        /// Register the "Save" attached property and the "OnSaveInvalidated" callback 
        /// </summary>
        public static readonly DependencyProperty SaveProperty
           = DependencyProperty.RegisterAttached("Save", typeof(bool), typeof(WindowPosition),
                new FrameworkPropertyMetadata(new PropertyChangedCallback(OnSaveInvalidated)));

        public static void SetSave(DependencyObject dependencyObject, bool enabled)
        {
            dependencyObject.SetValue(SaveProperty, enabled);
        }

        /// <summary>
        /// Called when Save is changed on an object.
        /// </summary>
        private static void OnSaveInvalidated(DependencyObject dependencyObject, DependencyPropertyChangedEventArgs e)
        {
            Window window = dependencyObject as Window;
            if (window != null)
            {
                if ((bool)e.NewValue)
                {
                    WindowPosition settings = new WindowPosition(window);
                    settings.Attach();
                }
            }
        }

        #endregion

        #region Protected Methods
        /// <summary>
        /// Load the Window Size Location and State from the settings object
        /// </summary>
        protected virtual void LoadWindowState()
        {
            this.Settings.Reload();
            if (this.Settings.Location != Rect.Empty)
            {
                this.window.Left = this.Settings.Location.Left;
                this.window.Top = this.Settings.Location.Top;
                this.window.Width = this.Settings.Location.Width;
                this.window.Height = this.Settings.Location.Height;
            }

            if (this.Settings.WindowState != WindowState.Maximized)
            {
                this.window.WindowState = this.Settings.WindowState;

                // Fix window position when resolution change
                this.window.Left = Math.Max(0, this.window.Left);
                this.window.Top = Math.Max(0, this.window.Top);

                double diffV = (this.window.Top + this.window.Height) - SystemParameters.VirtualScreenHeight;
                double diffH = (this.window.Left + this.window.Width) - SystemParameters.VirtualScreenWidth;

                // Fix window position
                if (diffH > 0 || diffV > 0)
                {
                    this.window.Left -= Math.Max(diffH, 0);
                    this.window.Top -= Math.Max(diffV, 0);
                }
            }
        }


        /// <summary>
        /// Save the Window Size, Location and State to the settings object
        /// </summary>
        protected virtual void SaveWindowState()
        {
            this.Settings.WindowState = this.window.WindowState;
            this.Settings.Location = this.window.RestoreBounds;
            this.Settings.Save();
        }
        #endregion

        #region Private Methods

        private void Attach()
        {
            if (this.window != null)
            {
                this.window.Closing += new CancelEventHandler(window_Closing);
                this.window.Initialized += new EventHandler(window_Initialized);
                this.window.Loaded += new RoutedEventHandler(window_Loaded);
            }
        }

        private void window_Loaded(object sender, RoutedEventArgs e)
        {
            if (this.Settings.WindowState == WindowState.Maximized)
            {
                this.window.WindowState = this.Settings.WindowState;
            }
        }

        private void window_Initialized(object sender, EventArgs e)
        {
            LoadWindowState();
        }

        private void window_Closing(object sender, CancelEventArgs e)
        {
            SaveWindowState();
        }
        #endregion

        #region Settings Property Implementation
        private WindowApplicationSettings windowApplicationSettings = null;

        protected virtual WindowApplicationSettings CreateWindowApplicationSettingsInstance()
        {
            return new WindowApplicationSettings(this);
        }

        [Browsable(false)]
        public WindowApplicationSettings Settings
        {
            get
            {
                if (windowApplicationSettings == null)
                {
                    this.windowApplicationSettings = CreateWindowApplicationSettingsInstance();
                }
                return this.windowApplicationSettings;
            }
        }
        #endregion
    }
}
