using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Input;
using System.Windows;
using System.Threading;

namespace FilePackager.Base.WPF
{
    public class AutoCursor : IDisposable
    {
        private Cursor _previousCursor = null;

        public AutoCursor(Cursor cursor)
        {
            if (Thread.CurrentThread.GetApartmentState() == ApartmentState.STA)
            {
                _previousCursor = Mouse.OverrideCursor;
                Mouse.OverrideCursor = cursor;
            }
        }

        ~AutoCursor() 
        {
            // Finalizer calls Dispose(false)
            Dispose(false);
        }

        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        protected virtual void Dispose(bool disposing)
        {
            if (disposing && Thread.CurrentThread.GetApartmentState() == ApartmentState.STA)
            {
                Mouse.OverrideCursor = _previousCursor;
            }
        }
    }
}
