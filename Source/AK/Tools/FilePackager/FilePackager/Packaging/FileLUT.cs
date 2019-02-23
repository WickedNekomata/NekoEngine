using System;
using System.Collections.Generic;
using System.Text;
using System.IO;

namespace FilePackager.Packaging
{
    /// <summary>
    /// File entry as it appears in the LUTs of the package header.
    /// </summary>
    internal class FileEntry
    {
        public FileEntry(UInt64 in_fileID, uint in_uLanguageID)
        {
            uFileID = in_fileID;
            uLanguageID = in_uLanguageID;
        }
        public FileEntry(UInt64 in_fileID, uint in_uBlockSize, uint in_uStartingBlock, ulong in_uFileSize, uint in_uLanguageID)
        {
            uFileID = in_fileID;
            uBlockSize = in_uBlockSize;
            uStartingBlock = in_uStartingBlock;
            uFileSize = in_uFileSize;
            uLanguageID = in_uLanguageID;
        }
        public UInt64 uFileID = 0;        // File identifier. 
        public uint uBlockSize = 0;     // Required alignment (in bytes).
        public ulong uFileSize = 0;     // File size in bytes. 
        public uint uStartingBlock = 0; // Start block, from beginning of DATA section. 
        public uint uLanguageID = 0;    // Language ID. AK_INVALID_LANGUAGE_ID if not language-specific. 

        static public uint SizeOfEntryOnDisk
        {
            get { return sizeof(uint) + sizeof(uint) + sizeof(uint) + sizeof(uint) + sizeof(uint);  }
        }
        static public uint SizeOfEntryOnDisk64
        {
            get { return sizeof(UInt64) + sizeof(uint) + sizeof(uint) + sizeof(uint) + sizeof(uint); }
        }
    };


    /// <summary>
    /// Definition of file LUTs written in the file package.
    /// File entries are sorted by file ID, then by language ID. 
    /// File ID + language ID couples are unique.
    /// Each file's block size is determined as they are added to the LUT.
    /// Usage: Create the LUT, Add() each file, then call Sort().
    /// Call Write() to dump to file package.
    /// </summary>
    internal class FileLUT
    {
        /// <summary>
        /// Extension of a FileEntry: Contains the path of the original file.
        /// </summary>
        internal class IncludedFile : FileEntry
        {
            public IncludedFile(UInt64 in_fileID, uint in_uLanguageID)
                : base(in_fileID, in_uLanguageID)
            {
            }
            public IncludedFile(UInt64 in_fileID, uint in_uBlockSize, uint in_uStartingBlock, ulong in_uFileSize, ushort in_uLanguageID, string in_szPath)
                : base(in_fileID, in_uBlockSize, in_uStartingBlock, in_uFileSize, in_uLanguageID)
            {
                m_szPath = in_szPath;
            }
            public string szPath
            {
                get { return m_szPath; }
            }
            public bool Included
            {
                get { return m_bIncluded; }
                set { m_bIncluded = value; }
            }
            private string m_szPath = "";
            private bool m_bIncluded = false;
        };

        /// <summary>
        /// Constructor. Initialized with the default desired alignment.
        /// If a particular file requires a different aligment, the actual alignment
        /// is computed as the least common multiple of both values.
        /// </summary>
        /// <param name="in_uDefaultBlockAlign">Default desired alignment.</param>
		public FileLUT( string in_rootPath, uint in_uDefaultBlockAlign, Type in_fileIdType )
        {
			m_rootPath = in_rootPath;
            m_uDefaultBlockAlign = in_uDefaultBlockAlign;
            m_fileIdType = in_fileIdType;
        }

        /// <summary>
        /// Add a file to the LUT.
        /// </summary>
        /// <param name="in_fileDesc">File descriptor.</param>
        /// <param name="in_mapLanguages">LanguageNameString-LanguageID hash.</param>
        /// <returns>Returns false if file is missing.</returns>
		public bool Add( AK.Wwise.InfoFile.FileDescriptorType in_fileDesc, Dictionary<string, uint> in_mapLanguages )
        {
			if ( in_fileDesc.Path.Length > 0 )
			{
				string szFullPath = System.IO.Path.Combine( m_rootPath, in_fileDesc.Path );

				if ( System.IO.File.Exists( szFullPath ) )
				{
					// Set all fields (ID, file size and language), but leave starting 
					// address (uBlock) to 0. In order to set this field, the offset 
					// of the data section must be known, and LUTs must be all considered
					// globally.

					// The INFO file should not have 2 files that have same ID and same language.
					ushort uLanguageID = (ushort)in_mapLanguages[in_fileDesc.Language];
					System.Diagnostics.Debug.Assert( !m_arFiles.Exists( delegate( IncludedFile in_searchedFile ) { return in_searchedFile.uFileID == in_fileDesc.Id && in_searchedFile.uLanguageID == uLanguageID; } ) );
					{
						// For file size:
						FileInfo fi = new FileInfo( szFullPath );
						// Block size: find least common multiple between desired global block size
						// and this file's minimum alignment requirement.
						m_arFiles.Add( new IncludedFile( in_fileDesc.Id, m_uDefaultBlockAlign, 0, (ulong)fi.Length, uLanguageID, szFullPath ) );
					}
					return true;
				}
			}

            return false;
        }

        public bool Add( FilePackageGenerator.Context.ExternalSourceInfo external, Dictionary<string, uint> in_mapLanguages)
        {
			if ( System.IO.File.Exists( external.Path ) )
			{
				// Set all fields (ID, file size and language), but leave starting 
				// address (uBlock) to 0. In order to set this field, the offset 
				// of the data section must be known, and LUTs must be all considered
				// globally.

				// The INFO file should not have 2 files that have same ID and same language.
				ushort uLanguageID = (ushort)in_mapLanguages["SFX"];
                UInt64 id = external.Id;

				System.Diagnostics.Debug.Assert( !m_arFiles.Exists( delegate( IncludedFile in_searchedFile ) { return in_searchedFile.uFileID == id && in_searchedFile.uLanguageID == uLanguageID; } ) );
				{
					// For file size:
                    FileInfo fi = new FileInfo(external.Path);

					// Block size: find least common multiple between desired global block size
					// and this file's minimum alignment requirement.
                    m_arFiles.Add(new IncludedFile(id, m_uDefaultBlockAlign, 0, (ulong)fi.Length, uLanguageID, external.Path));
				}
				return true;
			}

            return false;
        }
        
        public void Sort()
        {
            m_arFiles.Sort(new FileEntriesComparer());
        }
    
        /// <summary>
        /// Write LUT to file.
        /// NOTE: If there are 0 files, the method writes 4 zeroed bytes.
        /// </summary>
        /// <param name="in_writer">Binary writer.</param>
        public void Write(FilePackageWriter in_writer)
        {
            ulong uPositionBefore = in_writer.Position;

            in_writer.Write(NumFiles);
            foreach (FileEntry file in m_arFiles)
            {
                if (m_fileIdType == typeof(UInt32))
                    in_writer.Write((UInt32)file.uFileID);
                else
                    in_writer.Write(file.uFileID);
                in_writer.Write(file.uBlockSize);
                in_writer.Write((UInt32)file.uFileSize);
                in_writer.Write(file.uStartingBlock);
                in_writer.Write(file.uLanguageID);
            }
            
            System.Diagnostics.Debug.Assert(TotalSize == in_writer.Position - uPositionBefore);
        }

        public IncludedFile GetAt(int in_iIndex)
        {
            return m_arFiles[in_iIndex];
        }
        public int FindEntry(UInt64 in_id, uint in_languageID)
        {
            return m_arFiles.BinarySearch(new IncludedFile(in_id, in_languageID),new FileEntriesComparer());
        }

        /// <summary>
        /// Get the number of bytes needed to store the number of files of the LUT.
        /// </summary>
        public static uint LUTSizeSize
        {
            get { return sizeof(uint); }
        }

        /// <summary>
        /// Get the number of (unique) files referenced in the LUT.
        /// </summary>
        public uint NumFiles
        {
            get { return (uint)m_arFiles.Count; }
        }

        /// <summary>
        /// Allows foreach (IncludedFile) statements on FileLUTs.
        /// </summary>
        /// <returns></returns>
        public List<IncludedFile>.Enumerator GetEnumerator()
        {
            return m_arFiles.GetEnumerator();
        }

        /// <summary>
        /// Get the size of the LUT chunk.
        /// </summary>
        public uint TotalSize
        {
            get 
            {
                uint entrySize = m_fileIdType == typeof(UInt32) ? FileEntry.SizeOfEntryOnDisk : FileEntry.SizeOfEntryOnDisk64;
                return (uint)m_arFiles.Count * entrySize + LUTSizeSize; 
            }
        }

        public static uint GetApproximateTotalSize( uint bankStreamCount, uint externalCount )
        {
            return bankStreamCount * FileEntry.SizeOfEntryOnDisk+
                externalCount * FileEntry.SizeOfEntryOnDisk64
                + LUTSizeSize;
        }

        public class FileEntriesComparer: IComparer<IncludedFile>
        {
            // For sorting. Compares against fileID first, then language ID.
            public int Compare(IncludedFile x, IncludedFile y)
            {
                if (x.uFileID < y.uFileID)
                    return -1;
                else if (x.uFileID > y.uFileID)
                    return 1;
                else
                {
                    if (x.uLanguageID < y.uLanguageID)
                        return -1;
                    else if (x.uLanguageID > y.uLanguageID)
                        return 1;
                    else return 0;
                }
            }
        }

        private Type m_fileIdType;
		private string m_rootPath = "";
        private uint m_uDefaultBlockAlign = 0;
        private List<IncludedFile> m_arFiles = new List<IncludedFile>();
    };
}
