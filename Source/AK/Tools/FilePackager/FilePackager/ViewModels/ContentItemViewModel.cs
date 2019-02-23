using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using FilePackager.Base;
using System.IO;
using FilePackager.Packaging;

namespace FilePackager.ViewModels
{
	public enum FileType
	{
		SoundBank,
		StreamedFile,
        LooseMedia,
        StreamedFileRSX,    // PS3 - Deprecated. Keeping it for Persistance backward compatibility.
        ExternalSource,

        Unknown,
        Mixed
	}

	public class ContentItemViewModel : ViewModelBase
	{
		AK.Wwise.InfoFile.FileDescriptorType _fileDescriptor;
        FilePackageGenerator.Context.ExternalSourceInfo _externalSourceInfo;
        private long _size = -1;
        private UInt64 _id = 0;

		public ContentItemViewModel(AK.Wwise.InfoFile.FileDescriptorType fileDescriptor)
		{
			_fileDescriptor = fileDescriptor;
            _id = _fileDescriptor.Id;
        }

        public ContentItemViewModel(FilePackageGenerator.Context.ExternalSourceInfo externalSourceInfo)
        {
            _externalSourceInfo = externalSourceInfo;
            _id = _externalSourceInfo.Id;
        }
        
        public string FileName 
		{
            get { return _fileDescriptor != null ? _fileDescriptor.ShortName : _externalSourceInfo.Name; }
		}

        public long Size
        {
            get
            {
                if (_size == -1)
                {
                    string path = string.Empty;

                    if (FileType == FileType.SoundBank)
                    {
                        path = Path.Combine(ProjectViewModel.Current.SoundBanksRoot, _fileDescriptor.Path);
                    }
                    else if (FileType == FileType.StreamedFile || FileType == FileType.LooseMedia )
                    {
                        path = Path.Combine(ProjectViewModel.Current.SourceFilesRoot, _fileDescriptor.Path);
                    }
                    else if (FileType == FileType.ExternalSource)
                    {
                        path = _externalSourceInfo.Path;
                    }

                    System.IO.FileInfo fileInfo = new System.IO.FileInfo(path);
                    if (fileInfo.Exists)
                    {
                        _size = fileInfo.Length;
                    }
                    else
                    {
                        _size = 0;
                    }
                }

                return _size;
            }
        }

        public FileType FileType
        {
            get
            {
                if (_externalSourceInfo != null)
                {
                    return FileType.ExternalSource;
                }
                else
                {
                    Type fileType = _fileDescriptor.GetType();

                    if (fileType == typeof(AK.Wwise.InfoFile.SoundBanksInfoSoundBanksSoundBank))
                    {
                        return FileType.SoundBank;
                    }
                    else if (fileType == typeof(AK.Wwise.InfoFile.SoundBanksInfoStreamedFilesFile) ||
                        fileType == typeof(AK.Wwise.InfoFile.SoundBanksInfoSoundBanksSoundBankReferencedStreamedFilesFile))
                    {
                        return FileType.StreamedFile;
                    }
                    else if (fileType == typeof(AK.Wwise.InfoFile.SoundBanksInfoMediaFilesNotInAnyBankFile) ||
                        fileType == typeof(AK.Wwise.InfoFile.SoundBanksInfoSoundBanksSoundBankIncludedMemoryFilesFile) ||
                        fileType == typeof(AK.Wwise.InfoFile.SoundBanksInfoSoundBanksSoundBankExcludedMemoryFilesFile))
                    {
                        return FileType.LooseMedia;
                    }
                }
                throw new NotSupportedException();
            }
        }

        public string Language 
		{
			get 
            {
                return _fileDescriptor != null ? _fileDescriptor.Language : "SFX"; 
            }	
		}

        public UInt64 Id
        {
            get { return _id; }
        }

		public string References
		{
			get
			{
                IEnumerable<ReferenceManager.PackageReference> references = ProjectViewModel.Current.ReferenceManager.GetReferences(this);

                if (references != null)
                {
                    IEnumerable<PackageViewModel> packages = references.Select(p => p.Package).OrderBy(p => p.Name);
                    return packages.ToString(", ", p => p.Name);
                }
                return "";
			}
		}

        public AK.Wwise.InfoFile.FileDescriptorType FileDescriptor
        {
            get { return _fileDescriptor; }
        }

        public FilePackageGenerator.Context.ExternalSourceInfo ExternalSourceInfo
        {
            get { return _externalSourceInfo; }
        }

		internal void OnReferencesChanged()
		{
            OnPropertyChanged("References");
		}

        static readonly IEnumerable<ContentItemViewModel> _empty = new ContentItemViewModel[0];
        public IEnumerable<ContentItemViewModel> ReferencedStreamedFiles
        {
            get
            {
                IEnumerable<ContentItemViewModel> streamedFiles = _empty;

                AK.Wwise.InfoFile.SoundBanksInfoSoundBanksSoundBank soundBank = _fileDescriptor as AK.Wwise.InfoFile.SoundBanksInfoSoundBanksSoundBank;
                if (soundBank != null)
                {
                    streamedFiles = soundBank.ReferencedStreamedFiles.File
                        .Cast<AK.Wwise.InfoFile.SoundBanksInfoSoundBanksSoundBankReferencedStreamedFilesFile>()
                        .Select(file => ProjectViewModel.Current.GetContentItemLanguageSafe(file.Id, Language));
                }

                return streamedFiles;
            }
        }

        public IEnumerable<ContentItemViewModel> ReferencedLooseMedia
        {
            get
            {
                IEnumerable<ContentItemViewModel> looseMediaFiles = _empty;

                AK.Wwise.InfoFile.SoundBanksInfoSoundBanksSoundBank soundBank = _fileDescriptor as AK.Wwise.InfoFile.SoundBanksInfoSoundBanksSoundBank;
                if (soundBank != null)
                {
                    looseMediaFiles = ProjectViewModel.Current.ContentItems.Where( 
                        i => 
                            i.FileType == ViewModels.FileType.LooseMedia && 
                            soundBank.ExcludedMemoryFiles.File.Cast<AK.Wwise.InfoFile.FileDescriptorType>().Contains( i.FileDescriptor ) );
                }

                return looseMediaFiles;
            }
        }

        static readonly IEnumerable<AK.Wwise.InfoFile.FileDescriptorType> _emptyFd = new AK.Wwise.InfoFile.FileDescriptorType[0];
        public IEnumerable<AK.Wwise.InfoFile.FileDescriptorType> ExcludedMemoryFiles
        {
            get
            {
                IEnumerable<AK.Wwise.InfoFile.FileDescriptorType> mediaFiles = _emptyFd;
                AK.Wwise.InfoFile.SoundBanksInfoSoundBanksSoundBank soundBank = _fileDescriptor as AK.Wwise.InfoFile.SoundBanksInfoSoundBanksSoundBank;
                if (soundBank != null)
                    mediaFiles = soundBank.ExcludedMemoryFiles.File.SafeSelect(i => i as AK.Wwise.InfoFile.FileDescriptorType);
                return mediaFiles;
            }
        }

        public IEnumerable<AK.Wwise.InfoFile.FileDescriptorType> IncludedMemoryFiles
        {
            get
            {
                IEnumerable<AK.Wwise.InfoFile.FileDescriptorType> mediaFiles = _emptyFd;
                AK.Wwise.InfoFile.SoundBanksInfoSoundBanksSoundBank soundBank = _fileDescriptor as AK.Wwise.InfoFile.SoundBanksInfoSoundBanksSoundBank;
                if (soundBank != null)
                    mediaFiles = soundBank.IncludedMemoryFiles.File.SafeSelect(i => i as AK.Wwise.InfoFile.FileDescriptorType);
                return mediaFiles;
            }
        }
    }
}
