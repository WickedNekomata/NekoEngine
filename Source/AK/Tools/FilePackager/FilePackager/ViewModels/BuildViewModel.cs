using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using FilePackager.Base;
using System.Collections.ObjectModel;
using FilePackager.Packaging;
using System.ComponentModel;
using System.Windows.Threading;
using System.Windows;
using FilePackager.Commands;
using System.Windows.Input;
using System.Threading;

namespace FilePackager.ViewModels
{
    public enum CloseMode
    { 
        CloseOnlyWhenSuccess,
        AlwaysClose
    }

    public enum RunMode
    {
        Async,
        Blocking
    }

    public class BuildViewModel : ViewModelBase
    {
        public event EventHandler Finished;

        public class Message
        {
            public Severity Severity { get; set; }
            public string Text { get; set; }

            public override string ToString()
            {
                if (Severity != FilePackager.Packaging.Severity.Message)
                {
                    return Severity.ToString() + " - " + Text;
                }

                return Text;
            }
        }

        public enum Status
        { 
            InProgress,
            Stopping,
            Stopped,
            Finished
        }

        private DelegateCommand _stopCommand;
        private DelegateCommand _closeCommand;
        private DelegateCommand _copyToClipboard;

        private string _currentOperation;
        private string _currentPackage;
        private ObservableCollection<Message> _messages = new ObservableCollection<Message>();
        
        private int _progressPos = 0;
        private int _progressTotal = 0;

        private Severity _maxSeverityFound = Severity.None;

        private Status _currentStatus = Status.InProgress;

        private FilePackageGenerator.Context _context;

        private FilePackageGenerator _generator = null;
        private BackgroundWorker _worker = new BackgroundWorker();

        private CloseMode _closeMode = CloseMode.CloseOnlyWhenSuccess;

        public BuildViewModel(FilePackageGenerator.Context context)
        {
            CurrentOperation = Properties.Resources.PreparingBuild;

            _context = context;

            _worker.DoWork += Worker_DoWork;
            _worker.WorkerSupportsCancellation = true;
            _worker.RunWorkerCompleted += Worker_RunWorkerCompleted;
        }

        public override void Detach()
        {
            base.Detach();

            _worker.DoWork -= Worker_DoWork;
            _worker.RunWorkerCompleted -= Worker_RunWorkerCompleted;
            _worker = null;
        }

        public void Start(RunMode runMode)
        {
            if (runMode == RunMode.Async)
            {
                _worker.RunWorkerAsync();
            }
            else
            {
                Run();
            }
        }

        void Worker_RunWorkerCompleted(object sender, RunWorkerCompletedEventArgs e)
        {
            if (CurrentStatus == Status.Stopping)
            {
                CurrentStatus = Status.Stopped;
            }
            else
            {
                CurrentStatus = Status.Finished;
            }
        }

        void Worker_DoWork(object sender, DoWorkEventArgs e)
        {
            Run();
        }

        private void Run()
        {
            _generator = new FilePackageGenerator();
            _generator.LogMsg += Generator_LogMsg;
            _generator.GeneratingPackage += Generator_GeneratingPackage;
            _generator.PackageStep += Generator_PackageStep;

            _generator.Generate(_context);

            _generator.PackageStep -= Generator_PackageStep;
            _generator.GeneratingPackage -= Generator_GeneratingPackage;
            _generator.LogMsg -= Generator_LogMsg;
        }

        void Generator_PackageStep(object sender, FilePackageGenerator.PackageStepEventArgs e)
        {
            CurrentOperation = e.StepName;
            ++ProgressPos;
        }

        void Generator_GeneratingPackage(object sender, FilePackageGenerator.GeneratingPackageEventArgs e)
        {
            CurrentPackage = e.PackageName;
            ProgressPos = 0;
            ProgressTotal = e.NumSteps;
        }

        void Generator_LogMsg(object sender, FilePackageGenerator.LogMsgEventArgs e)
        {
            if (e.Severity > MaxSeverityFound)
            {
                MaxSeverityFound = e.Severity;
            }

            if (Application.Current != null && 
                Thread.CurrentThread.GetApartmentState() != ApartmentState.STA)
            {
                Application.Current.Dispatcher.BeginInvoke(DispatcherPriority.Normal,
                    (Action)delegate
                    {
                        Messages.Add(new Message() { Text = e.Msg, Severity = e.Severity });
                    });
            }
            else
            {
                Messages.Add(new Message() { Text = e.Msg, Severity = e.Severity });
            }
        }

        public CloseMode CloseMode
        {
            get { return _closeMode; }
            set { _closeMode = value; }
        }

        public string CurrentOperation
        {
            get { return _currentOperation; }
            set 
            {
                _currentOperation = value;
                OnPropertyChanged("CurrentOperation");
            }
        }

        public string CurrentPackage
        {
            get { return _currentPackage ; }
            set
            {
                _currentPackage = value;
                OnPropertyChanged("CurrentPackage");
            }
        }

        public ObservableCollection<Message> Messages
        {
            get { return _messages; }
        }
        
        public int ProgressPos
        {
            get { return _progressPos; }
            set 
            { 
                _progressPos = value;
                OnPropertyChanged("ProgressPos");
            }
        }

        public int ProgressTotal
        {
            get { return _progressTotal; }
            set
            {
                if (value != _progressTotal)
                {
                    _progressTotal = value;
                    OnPropertyChanged("ProgressTotal");
                }
            }
        }

        public bool HasWarningOrError
        {
            get { return _maxSeverityFound >= Severity.Warning; }
        }

        public Severity MaxSeverityFound
        {
            get { return _maxSeverityFound; }
            set
            {
                _maxSeverityFound = value;
                OnPropertyChanged("MaxSeverityFound");
                OnPropertyChanged("HasWarningOrError");
            }
        }
        
        public Status CurrentStatus
        {
            get { return _currentStatus; }
            set 
            { 
                _currentStatus = value;

                if (CloseMode == CloseMode.AlwaysClose &&
                    (_currentStatus == Status.Finished || _currentStatus == Status.Stopped))
                {
                    RaiseFinished();
                }
                else if ( CloseMode == CloseMode.CloseOnlyWhenSuccess && 
                    _currentStatus == Status.Finished && 
                    !HasWarningOrError)
                {
                    RaiseFinished();
                }

                ((DelegateCommand)StopCommand).RaiseCanExecuteChanged();
                ((DelegateCommand)CloseCommand).RaiseCanExecuteChanged();

                OnPropertyChanged("CurrentStatus");
                OnPropertyChanged("CurrentStatusText");
            }
        }

        public string CurrentStatusText
        {
            get
            {
                switch (CurrentStatus)
                {
                    case Status.InProgress:
                        return "In Progress...";
                    case Status.Stopping:
                        return "Stopping...";
                    case Status.Finished:
                        return "Finished";
                    case Status.Stopped:
                        return "Stopped";
                }
                return "";
            }
        }

        private void RaiseFinished()
        {
            if (Finished != null)
            {
                Finished(this, EventArgs.Empty);
            }
        }

        public ICommand StopCommand
        {
            get { return CommandManagerHelper.CreateOnceCommand(ref _stopCommand, Stop, CanStop); }
        }

        public ICommand CloseCommand
        {
            get { return CommandManagerHelper.CreateOnceCommand(ref _closeCommand, Close, CanClose); }
        }

        public ICommand CopyToClipboardCommand
        {
            get { return CommandManagerHelper.CreateOnceCommand(ref _copyToClipboard, CopyToClipboard, null); }
        }
        

        public void Stop()
        {
            _generator.Stop();
            CurrentStatus = Status.Stopping;
        }

        private bool CanStop()
        {
            return CurrentStatus == Status.InProgress;
        }

        public void Close()
        {
            RaiseFinished();
        }

        private bool CanClose()
        {
            return CurrentStatus == Status.Finished || CurrentStatus == Status.Stopped;
        }

        private void CopyToClipboard()
        {
            StringBuilder output = new StringBuilder();

            foreach (Message message in Messages)
            {
                output.Append(message.Severity.ToString());
                output.Append(": ");
                output.Append(message.Text); 
                output.Append("\r\n");
            }

            Clipboard.SetText(output.ToString());
        }
    }
}
