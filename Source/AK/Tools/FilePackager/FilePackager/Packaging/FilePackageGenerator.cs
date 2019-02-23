using System;
using System.Collections.Generic;
using System.Text;
using System.IO;
using System.Linq;
using FilePackager.Base;

namespace FilePackager.Packaging
{
    /// <summary>
    /// ProgressNotificationsDispatcher interface. The FilePackageGenerator 
    /// implements this interface to allow other objects to dispatch notifications 
    /// to progress listeners.
    /// </summary>
    interface IProgressNotificationsDispatcher
    {
        /// <summary>
        /// Send a message about the generation progress.
        /// </summary>
        /// <param name="in_szMsg"></param>
        void NotifyLogMsg(string in_szMsg, Severity severity);
    }

    public class NamedAction
    {
        public NamedAction(string name, Action action)
        {
            Name = name;
            Action = action;
        }
        public string Name { get; set; }
        public Action Action { get; set; }
    }

    /// <summary>
    /// FilePackageGenerator: Generates a file package based on an InfoFile and some
    /// settings (PackageSettings).
    /// 
    /// Packaged file header format: 
    /// IMPORTANT: KEEP IN SYNC WITH /SDK/Samples/SoundEngine/Common/PackagedFileDefs.h
    /// 
    /// 'AKPK' (4 bytes)
    /// Header size, excluding the header chunk definition (-8 bytes) (number of bytes) (4 bytes)
    /// Version (4 bytes)
    /// Language names map size (4 bytes) 
    /// SoundBank titles map size (4 bytes) 
    /// SoundBank files LUT size (4 bytes) 
    /// Streamed audio files LUT size (4 bytes) 
    /// Language names map (variable) (sorted by strings)
    /// SoundBank titles map (variable) (sorted by strings)
    /// SoundBank files LUT (variable) (sorted by ID)
    /// Streamed files LUT (variable) (sorted by ID)
    /// 
    /// String maps:
    /// Number of strings (4 bytes)
    /// Array of StringEntry (NSTRINGS * sizeof(StringEntry))
    /// Concatenated strings (Sum((NumChars(i)+Null) * sizeof(WCHAR)))
    /// 
    /// File LUT
    /// Number of files (4 bytes)
    /// Array of FileEntry (NFILES * sizeof(FileEntry))
    /// </summary>
    public class FilePackageGenerator : IProgressNotificationsDispatcher
    {
        const uint AK_INVALID_LANGUAGE_ID = 0;

        // File package generator events.

        public class GeneratingPackageEventArgs : EventArgs
        {
            public string PackageName { get; set; }
            public int NumSteps { get; set; }
        }

        public class PackageStepEventArgs : EventArgs
        {
            public string StepName { get; set; }
        }

        /// <summary>
        /// Custom event definition for LogMsg event.
        /// Provides the message.
        /// </summary>
        public class LogMsgEventArgs : EventArgs
        {
            public string Msg { get; set; }
            public Severity Severity { get; set; }
        }

        /// <summary>
        /// Occurs when generating a package.
        /// </summary>
        public event EventHandler<GeneratingPackageEventArgs> GeneratingPackage;
        
        /// <summary>
        /// Occurs when package step is done.
        /// </summary>
        public event EventHandler<PackageStepEventArgs> PackageStep;

        /// <summary>
        /// Register to this event to be notified of messages about the generation process.
        /// </summary>
        public event EventHandler<LogMsgEventArgs> LogMsg;

        public class Context
        {
            public Context() { Global = new GlobalInfo(); }
            public class GlobalInfo
            {
                public AK.Wwise.InfoFile.SoundBanksInfoBasePlatform BasePlatform { get; set; }
                public string SoundBanksRoot { get; set; }
                public string SourceFilesRoot { get; set; }
            }

            public class PackageItem
            {
                public PackageItem(AK.Wwise.InfoFile.FileDescriptorType in_descriptor, ExternalSourceInfo in_externalSourceInfo)
                {
                    Descriptor = in_descriptor;
                    ExternalSourceInfo = in_externalSourceInfo;
                }

                // Can be either one of the two:
                public AK.Wwise.InfoFile.FileDescriptorType Descriptor { get; set; }
                public ExternalSourceInfo ExternalSourceInfo { get; set; }
            }

            public class PackageInfo
            {
                public string FilePackageFilename { get; set; }
                public uint BlockSize { get; set; }
                public IEnumerable<PackageItem> Files { get; set; }
                public IEnumerable<string> MissingFiles { get; set; }
                public IEnumerable<string> MissingFromPackageFiles { get; set; }
            }

            public class ExternalSourceInfo
            {
                public string Name { get; set; }
                public string Path { get; set; }
                public UInt64 Id
                { 
                    get { return FNVHash64.Compute(Name.ToLower()); }
                }
            }

            public GlobalInfo Global { get; set; }
            public IEnumerable<PackageInfo> Packages { get; set; }
        }

        private class Results
        {
            public Results()
            {
                HasMissingFiles = false;
            }

            public bool HasMissingFiles { get; set; }
        }

        /// <summary>
        /// Generate a file package.
        /// </summary>
        /// <param name="in_soundbanksInfo">Soundbanks data model.</param>
        /// <param name="in_settings">Generation settings.</param>
        /// <param name="in_szOutputFileName">Full path of the file package to be created.</param>
        /// <returns>Returns true when no files are missing.</returns>
		public bool Generate( Context context )
        {
            Results results = new Results();

            foreach (Context.PackageInfo packageInfo in context.Packages)
            {
                if (StopRequested)
                    break;

                Generate(context.Global, packageInfo, results);
            }

            return results.HasMissingFiles;
        }
        
        private void Generate( Context.GlobalInfo globalInfo, Context.PackageInfo packageInfo, Results results )
        {
            // Open output file.
            try
            {
                // Make sure the target directory exist
                string directory = Path.GetDirectoryName(packageInfo.FilePackageFilename);
                if (!Directory.Exists(directory))
                {
                    Directory.CreateDirectory(directory);
                }

                using (FileStream file = new FileStream(packageInfo.FilePackageFilename, FileMode.Create))
                {
                    // Create the writer for data.
                    FilePackageWriter.Endianness eEndianness;
                    switch (globalInfo.BasePlatform)
                    {
                        case AK.Wwise.InfoFile.SoundBanksInfoBasePlatform.Windows:
                        case AK.Wwise.InfoFile.SoundBanksInfoBasePlatform.Mac:
                        case AK.Wwise.InfoFile.SoundBanksInfoBasePlatform.VitaSW:
                        case AK.Wwise.InfoFile.SoundBanksInfoBasePlatform.VitaHW:
                        case AK.Wwise.InfoFile.SoundBanksInfoBasePlatform.Item3DS:
                        case AK.Wwise.InfoFile.SoundBanksInfoBasePlatform.iOS:
                        case AK.Wwise.InfoFile.SoundBanksInfoBasePlatform.Android:
                        case AK.Wwise.InfoFile.SoundBanksInfoBasePlatform.PS4:
                        case AK.Wwise.InfoFile.SoundBanksInfoBasePlatform.XboxOne:
                        case AK.Wwise.InfoFile.SoundBanksInfoBasePlatform.Linux:
                        case AK.Wwise.InfoFile.SoundBanksInfoBasePlatform.WindowsPhone:
                        case AK.Wwise.InfoFile.SoundBanksInfoBasePlatform.Emscripten:
                        case AK.Wwise.InfoFile.SoundBanksInfoBasePlatform.Switch:
                            eEndianness = FilePackageWriter.Endianness.LittleEndian;
                            break;
                        default:
                            eEndianness = FilePackageWriter.Endianness.BigEndian;
                            break;
                    }

                    FilePackageWriter writer = new FilePackageWriter(file, eEndianness);

                    // Generate the file package.
                    IEnumerable<NamedAction> actions = GeneratePackage(globalInfo, packageInfo, writer, results);

                    // Notify about this package
                    if (GeneratingPackage != null)
                    {
                        GeneratingPackage(this, new GeneratingPackageEventArgs()
                        {
                            NumSteps = actions.Count(),
                            PackageName = packageInfo.FilePackageFilename
                        });
                    }

                    // Notify about missing files
                    foreach (string missingFile in packageInfo.MissingFiles)
                    {
                        NotifyLogMsg("Package '" + packageInfo.FilePackageFilename + "' contains a missing file: " + missingFile, Severity.Warning);
                    }

                    // Notify about missing from package files
                    foreach (string missingFile in packageInfo.MissingFromPackageFiles)
                    {
                        NotifyLogMsg("Layout for package '" + packageInfo.FilePackageFilename + "' contains a file not present in package: " + missingFile, Severity.Warning);
                    }

                    // Execute the actions
                    foreach (NamedAction action in actions)
                    {
                        if (StopRequested)
                            break;

                        // Notify about this step
                        if (PackageStep != null)
                        {
                            PackageStep(this, new PackageStepEventArgs() { StepName = action.Name });
                        }

                        // Execute the step
                        action.Action();
                    }

                    writer.Close();
                    file.Close();
                }
            }
            catch (Exception e)
            {
                NotifyLogMsg("Error writing package \"" + packageInfo.FilePackageFilename + "\":" + e.Message, Severity.Error);
            }
        }

        
        /// <summary>
        /// Generate the file package.
        /// Creates the package header:
        /// - Header
        /// - Map of language strings
        /// - Map of soundbank titles
        /// - Soundbank files LUT
        /// - Streamed audio files LUT
        /// Writes the header to file.
        /// Concatenates files referenced in the LUTs.
        /// </summary>
        /// <param name="in_soundbanksInfo">Soundbank data model.</param>
        /// <param name="in_settings">Generation settings.</param>
        /// <param name="in_writer">Binary writer.</param>
        /// <returns>Returns true when no files are missing.</returns>
        private IEnumerable<NamedAction> GeneratePackage(
            Context.GlobalInfo globalInfo, 
            Context.PackageInfo packageInfo, 
            FilePackageWriter in_writer, 
            Results results)
        {
            List<NamedAction> actions = new List<NamedAction>();

            // Header chunk.
            Header header = new Header();

            // Language names map.
            // NOTE: As of Wwise 2009.1, language names are stored as ANSI strings when not on Windows (sync with type AkOSChar).
            bool bLanguageMapUsesAsciiStrings = true;
            switch (globalInfo.BasePlatform)
            {
                case AK.Wwise.InfoFile.SoundBanksInfoBasePlatform.Windows:
                case AK.Wwise.InfoFile.SoundBanksInfoBasePlatform.XboxOne:
                    bLanguageMapUsesAsciiStrings = false;
                    break;
                default:
                    bLanguageMapUsesAsciiStrings = true;
                    break;
            }

            IEnumerable<AK.Wwise.InfoFile.FileDescriptorType> descriptors = packageInfo.Files.Where(f => f.Descriptor != null).Select(f => f.Descriptor);
            IEnumerable<FilePackageGenerator.Context.ExternalSourceInfo> externals = packageInfo.Files.Where(f => f.ExternalSourceInfo != null).Select(f => f.ExternalSourceInfo);

            Dictionary<string, uint> mapLanguageIDs = FindAllLanguages(descriptors);
            LanguagesMap langMap = new LanguagesMap(mapLanguageIDs, bLanguageMapUsesAsciiStrings);

            // Add Banks files to LUT.
            FileLUT banksLUT = new FileLUT(globalInfo.SoundBanksRoot, packageInfo.BlockSize,typeof(UInt32));

            foreach (AK.Wwise.InfoFile.FileDescriptorType soundbank in descriptors.OfType<AK.Wwise.InfoFile.SoundBanksInfoSoundBanksSoundBank>())
            {
                if (!banksLUT.Add(soundbank, mapLanguageIDs))
                {
                    NotifyLogMsg("Missing soundbank: " + soundbank.ShortName + " (" + soundbank.Path + ")", Severity.Warning);
                    results.HasMissingFiles = true;
                }
            }
            banksLUT.Sort();
            
            // Add Steamed files to LUT.
            FileLUT streamsLUT = new FileLUT(globalInfo.SourceFilesRoot, packageInfo.BlockSize, typeof(UInt32));

            foreach (AK.Wwise.InfoFile.FileDescriptorType stream in descriptors.Where( fd => ( fd is AK.Wwise.InfoFile.SoundBanksInfoStreamedFilesFile || 
                                                                                               fd is AK.Wwise.InfoFile.SoundBanksInfoMediaFilesNotInAnyBankFile ) ) )
            {
                if (!streamsLUT.Add(stream, mapLanguageIDs))
                {
                    NotifyLogMsg("Missing streamed or loose media file: " + stream.ShortName + " (" + stream.Path + ")", Severity.Warning);
                    results.HasMissingFiles = true;
                }
            }
            streamsLUT.Sort();

            // Add External Source files to LUT.
            FileLUT externalLUT = new FileLUT(globalInfo.SourceFilesRoot, packageInfo.BlockSize, typeof(UInt64));

            foreach (FilePackageGenerator.Context.ExternalSourceInfo external in externals)
            {
                if (!externalLUT.Add(external, mapLanguageIDs))
                {
                    NotifyLogMsg("Missing external file: " + external.Name + " (" + external.Path + ")", Severity.Warning);
                    results.HasMissingFiles = true;
                }
            }
            externalLUT.Sort();

            // Find the header size.
            uint uHeaderSize =
                BaseHeaderSize +
                langMap.TotalSize +
                banksLUT.TotalSize +
                streamsLUT.TotalSize +
                externalLUT.TotalSize;
            
            // Prepare files for ordered concatenation.
            FileOrganizer organizer = new FileOrganizer();
            organizer.AddLUT(FileOrganizer.FileCategory.SoundBank, banksLUT);
            organizer.AddLUT(FileOrganizer.FileCategory.MediaFile, streamsLUT);
            organizer.AddLUT(FileOrganizer.FileCategory.ExternalSource, externalLUT);
            organizer.OrganizeFiles(uHeaderSize, packageInfo.Files, mapLanguageIDs, this);

            // Set header size.
            header.HeaderSize = uHeaderSize;


            // Write to output file:
            actions.Add( new NamedAction("Writing header", delegate()
            {
                // Header.
                header.Write(in_writer);
                in_writer.Write(langMap.TotalSize);
                in_writer.Write(banksLUT.TotalSize);
                in_writer.Write(streamsLUT.TotalSize);
                in_writer.Write(externalLUT.TotalSize);

                langMap.Write(in_writer);
                banksLUT.Write(in_writer);
                streamsLUT.Write(in_writer);
                externalLUT.Write(in_writer);
            }));

            // Concatenated files.
            return actions.Concat( organizer.ConcatenateFiles(in_writer, this));
        }

        static public uint BaseHeaderSize
        {
            get
            {
                return Header.SizeOnDisk +
                    StringMap.MapSizeSize +
                    FileLUT.LUTSizeSize + // bank
                    FileLUT.LUTSizeSize + // stream
                    FileLUT.LUTSizeSize; // external
            }
        }

        /// <summary>
        /// Data model helper: Search the model for all languages, create an ID dynamically for each language.
        /// </summary>
        /// <param name="in_infoDOM">Data model.</param>
        /// <returns>A hash [LanguageNameString, GeneratedLanguageID]</returns>
        static internal Dictionary<string, uint> FindAllLanguages(IEnumerable<AK.Wwise.InfoFile.FileDescriptorType> descriptors )
        {
            Dictionary<string, uint> mapLanguages = new Dictionary<string, uint>();
            uint uID = AK_INVALID_LANGUAGE_ID;
            mapLanguages.Add("SFX", uID);

            // Search languages in streamed files.
            foreach (AK.Wwise.InfoFile.FileDescriptorType streamedFile in descriptors.Where(fd => (fd is AK.Wwise.InfoFile.SoundBanksInfoStreamedFilesFile || 
                                                                                                   fd is AK.Wwise.InfoFile.SoundBanksInfoMediaFilesNotInAnyBankFile ) ) )
            {
                string szLanguage = streamedFile.Language;
                if (!mapLanguages.ContainsKey(szLanguage))
                {
                    ++uID;
                    mapLanguages.Add(szLanguage, uID);
                }
            }

            // Search languages in soundbanks.
            foreach (AK.Wwise.InfoFile.SoundBanksInfoSoundBanksSoundBank soundBank in descriptors.OfType<AK.Wwise.InfoFile.SoundBanksInfoSoundBanksSoundBank>())
            {
                string szLanguage = soundBank.Language;
                if (!mapLanguages.ContainsKey(szLanguage))
                {
                    ++uID;
                    mapLanguages.Add(szLanguage, uID);
                }
            }

            return mapLanguages;
        }

        /// <summary>
        /// ProgressNotificationsDispatcher implementation.
        /// Dispatches the LogMsg event.
        /// </summary>
        /// <param name="in_szMsg">Message</param>
        public void NotifyLogMsg(string in_szMsg, Severity severity)
        {
            if (LogMsg != null)
                LogMsg(this, new LogMsgEventArgs() { Msg = in_szMsg, Severity = severity });
        }

        /// <summary>
        /// Progress.StopRequestedEventHandler.
        /// Handles "abort generation" events.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        public void Stop()
        {
            m_bStopRequested = true;
        }

        /// <summary>
        /// Returns true if someone requested that the generation be stopped.
        /// </summary>
        public bool StopRequested
        {
            get { return m_bStopRequested; }
        }

        private bool m_bStopRequested = false;
    }

    public enum Severity: int
    {
        None,
        Message,
        Warning,
        Error
    }
}
