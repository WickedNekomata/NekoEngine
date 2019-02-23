using System;
using System.Collections.Generic;
using System.Text;

namespace FilePackager.Packaging
{
    /// <summary>
    /// File organizer: Once the LUTs have been created, one list of files to
    /// concatenate is created through the FileOrganizer. The files of all LUTs can
    /// be placed in any order. When this is done, the (original) entries of all
    /// LUTs are updated with the uStartingBlock. Then the actual files can be concatenated
    /// to the file package.
    /// Usage: Once LUTs are created and the offset of the data section is known,
    /// AddLUT() each LUT and call OrganizeFiles(). This will fill up the organizer's
    /// internal list and update all entries' uStartingBlock.
    /// When the header is written to file, call ConcatenateFiles() to copy the content
    /// of each file referenced by the file entries to the file package.
    /// </summary>
    internal class FileOrganizer
    {
        public enum FileType
        {
            StreamedAudio,
            LooseMedia,
            SoundBank,
            ExternalSource,
        }

        public enum FileCategory
        {
            MediaFile,
            SoundBank,
            ExternalSource,

            Unknown
        }

        public FileCategory CategoryForType(FileType in_eType)
        {
            switch (in_eType) 
            {
                case FileType.StreamedAudio:
                case FileType.LooseMedia:
                    return FileCategory.MediaFile;
                case FileType.SoundBank:
                    return FileCategory.SoundBank;
                case FileType.ExternalSource:
                    return FileCategory.ExternalSource;
            }

            return FileCategory.Unknown;
        }

        public FileOrganizer()
        {
        }

        public void AddLUT(FileCategory in_eType, FileLUT fileLUTs)
        {
            m_arFileLUTs[in_eType] = fileLUTs;
        }

        public void OrganizeFiles(
            ulong in_uDataOffset,
            IEnumerable<FilePackageGenerator.Context.PackageItem> in_ordered,
            Dictionary<string, uint> in_mapLanguages, 
            IProgressNotificationsDispatcher in_notifDispatcher)
        {
            // Build list of files based on specified file order. 
            if (in_ordered != null)
            {
                foreach (FilePackageGenerator.Context.PackageItem item in in_ordered)
                {
                    string language = "SFX";
                    string name = string.Empty;
                    UInt64 itemId = 0;
                    FileType fileType = FileType.SoundBank;
                    if (item.ExternalSourceInfo != null)
                    {
                        fileType = FileType.ExternalSource;
                        itemId = item.ExternalSourceInfo.Id;
                        name = item.ExternalSourceInfo.Name;
                    }
                    else if (item.Descriptor != null)
                    {
                        language = item.Descriptor.Language;
                        itemId = item.Descriptor.Id;
                        name = item.Descriptor.ShortName;
                        if ( item.Descriptor.GetType() == typeof(AK.Wwise.InfoFile.SoundBanksInfoStreamedFiles) )
                            fileType = FileType.StreamedAudio;
                        else if ( item.Descriptor.GetType() == typeof(AK.Wwise.InfoFile.SoundBanksInfoMediaFilesNotInAnyBank) )
                            fileType = FileType.LooseMedia;
                    }

                    // Select proper LUT.
                    FileLUT lut = m_arFileLUTs[CategoryForType(fileType)];
                                        
                    // Find file (binary search: files in LUTs are sorted in ID/LanguageID.
                    uint uLanguageID;
                    if (in_mapLanguages.TryGetValue(language, out uLanguageID))
                    {
                        int iFileIndex = lut.FindEntry(itemId, uLanguageID);
                        if (iFileIndex >= 0)
                        {
                            FileLUT.IncludedFile file = lut.GetAt(iFileIndex);

                            // File was laid out. 
                            // Set starting block, mark as ready.
                            AddOrganizedFile(file, ref in_uDataOffset);
                        }
                    }
                    else
                    {
                        // Invalid language.
                        in_notifDispatcher.NotifyLogMsg("WARNING: File " + name + " specified in the layout has invalid language + " + language + ".", Severity.Warning);
                    }
                }
            }

            // Default logic: add files of all LUTs in order.

            // Set each file's starting block.
            foreach (FileLUT lut in m_arFileLUTs.Values)
            {
                foreach (FileLUT.IncludedFile file in lut)
                {
                    if (!file.Included)
                    {
                        AddOrganizedFile(file, ref in_uDataOffset);
                    }
                }
            }
        }

        public IEnumerable<NamedAction> ConcatenateFiles(FilePackageWriter in_writer, IProgressNotificationsDispatcher in_notifDispatcher)
        {
            List<NamedAction> actions = new List<NamedAction>();

            foreach (OrganizedFile file in m_arOrganizedFileEntries)
            {
                string path = file.szPath;
                uint blockSize = file.uBlockSize;

                actions.Add(new NamedAction("Writing " + path, delegate()
                {
                    // Add padding so that next file falls on a block boundary.
                    PadToBlock(in_writer, blockSize, in_writer.Position);

                    // At this point we know the file exists. 
                    System.Diagnostics.Debug.Assert(path.Length > 0 && System.IO.File.Exists(path));

                    in_notifDispatcher.NotifyLogMsg("Packing: " + path,Severity.Message);

                    // Copy file.
                    in_writer.Write(System.IO.File.ReadAllBytes(path));
                }));
            }

            return actions;
        }

        private void AddOrganizedFile(FileLUT.IncludedFile in_file, ref ulong io_uDataOffset)
        {
            System.Diagnostics.Debug.Assert(!in_file.Included);
            io_uDataOffset += ComputePaddingSize(in_file.uBlockSize, io_uDataOffset);
            // Starting block is expressed in terms of this file's own block size.
            System.Diagnostics.Debug.Assert(io_uDataOffset % in_file.uBlockSize == 0);
            in_file.uStartingBlock = (uint)(io_uDataOffset / in_file.uBlockSize);
            m_arOrganizedFileEntries.Add(new OrganizedFile(in_file.szPath, in_file.uBlockSize));
            in_file.Included = true;
            io_uDataOffset += in_file.uFileSize;
        }

        /// <summary>
        /// File helper: Compute padding size based on the required alignment and current offset.
        /// </summary>
        /// <param name="in_uBlockSize">Required alignment.</param>
        /// <param name="in_uOffset">Current offset.</param>
        /// <returns>Number of bytes to be written to file to meet the required alignment.</returns>
        internal static uint ComputePaddingSize(uint in_uBlockSize, ulong in_uOffset)
        {
            return (uint)((in_uBlockSize * (uint)((in_uOffset + in_uBlockSize - 1) / in_uBlockSize)) - in_uOffset);
        }

        /// <summary>
        /// File helper: Write a certain amount of padding zeros to file.
        /// </summary>
        /// <param name="in_writer">Binary writer.</param>
        /// <param name="in_uPadSize">Number of zeroed bytes to be written.</param>
        internal static void Pad(FilePackageWriter in_writer, uint in_uPadSize)
        {
            if (in_uPadSize > 0)
            {
                byte[] padding = new byte[in_uPadSize];
                in_writer.Write(padding);
            }
        }

        /// <summary>
        /// File helper: Write padding zeros to file to meet the required alignment.
        /// </summary>
        /// <param name="in_writer">Binary writer.</param>
        /// <param name="in_uBlockSize">Required alignment.</param>
        /// <param name="in_uOffset">Current offset.</param>
        internal static void PadToBlock(FilePackageWriter in_writer, uint in_uBlockSize, ulong in_uOffset)
        {
            Pad(in_writer, ComputePaddingSize(in_uBlockSize, in_uOffset));
        }

        struct OrganizedFile
        {
            public OrganizedFile( string in_szPath, uint in_uBlockSize )
            {
                szPath = in_szPath;
                uBlockSize = in_uBlockSize;
            }

            public string szPath;
            public uint uBlockSize;
        }

        private Dictionary<FileCategory, FileLUT> m_arFileLUTs = new Dictionary<FileCategory, FileLUT>();
        private List<OrganizedFile> m_arOrganizedFileEntries = new List<OrganizedFile>();
    };
}
