using System;
using System.Collections.Generic;
using System.Text;

namespace FilePackager.Packaging
{
    /// <summary>
    /// String entry as it appears in the string maps of the package header.
    /// </summary>
    internal class StringEntry
    {
        public StringEntry(uint in_ID)
        {
            m_ID = in_ID;
        }

        static public uint SizeOfEntryOnDisk
        {
            get { return 8; }
        }

        /// <summary>
        /// Byte offset of the beginning of the string relative to the beginning of the map. 
        /// </summary>
        public uint Offset
        {
            get { return m_uOffset; }
            set { m_uOffset = value; }
        }
        
        /// <summary>
        /// Associated ID.
        /// </summary>
        public uint ID
        {
            get { return m_ID; }
        }
        private uint m_ID = 0;
        private uint m_uOffset = 0;
    };


    /// <summary>
    /// String map to be stored in the file package header.
    /// String maps have the following structure:
    /// 
    /// - Total string map size in bytes (4 bytes)
    /// 
    /// - BEGINNING OF MAP: Number of strings (4 bytes)
    /// - Array of StringEntry: [ID (4 bytes), Location of string (4 bytes)]
    /// - All null-terminated strings, concatenated, in alphabetical order.
    /// 
    /// Note: The location of strings (StringEntry.uOffset) is represented by 
    /// the number of bytes between the string and the BEGINNING OF MAP.
    /// </summary>
    internal class StringMap
    {
        /// <summary>
        /// StringMap constructor.
        /// </summary>
        /// <param name="in_bPrintAsciiStrings">All strings are converted to ascii when written to disk if true. CharSize returns appropriate character size</param>
        public StringMap(bool in_bPrintAsciiStrings)
        {
            m_bPrintAsciiStrings = in_bPrintAsciiStrings;
        }

        /// <summary>
        /// Add a StringInfo to the map only if the string does not exist. 
        /// Strings are converted to lower case before being added.
        /// </summary>
        /// <param name="in_stringInfo">String info to add to the map.</param>
        protected void AddString(string in_string, StringEntry in_stringEntry)
        {
            in_string = in_string.ToLower();
            m_hashStrings[in_string] = in_stringEntry;
        }

        /// <summary>
        /// Write string map to file.
        /// </summary>
        /// <param name="in_writer">Binary writer.</param>
        public void Write(FilePackageWriter in_writer)
        {
            ulong uPositionBefore = in_writer.Position;

            in_writer.Write((uint)m_hashStrings.Count);
            List<string> sortedKeys = GetSortedKeys();

            foreach (string szKey in sortedKeys)
            {
                in_writer.Write(m_hashStrings[szKey].Offset);
                in_writer.Write(m_hashStrings[szKey].ID);
            }
            foreach (string szKey in sortedKeys)
            {
                in_writer.Write(szKey,m_bPrintAsciiStrings);
            }

            // Pad for proper alignment of the rest of the data
            FileOrganizer.PadToBlock(in_writer, MapDataSizeAlignment, m_uTotalMapSize);

            System.Diagnostics.Debug.Assert(TotalSize == in_writer.Position - uPositionBefore);
        }

        /// <summary>
        /// Get the number of bytes needed to store the map size.
        /// </summary>
        static public uint MapSizeSize
        {
            get { return sizeof(uint); }
        }

        /// <summary>
        /// This map's data size must be a multiple of this value,
        /// so the data that follows is properly aligned.
        /// </summary>
        public uint MapDataSizeAlignment
        {
            get { return 4; }
        }

        /// <summary>
        /// Get the total number of bytes taken by the string map in the file package header.
        /// </summary>
        public uint TotalSize
        {
            get
            {
                // Make sure the data size is a multiple of MapDataSizeAlignment
                return m_uTotalMapSize + FileOrganizer.ComputePaddingSize(MapDataSizeAlignment, m_uTotalMapSize);
            }
        }

        public static uint ApproximateTotalSize
        {
            get
            {
                return 32;
            }
        }

        /// <summary>
        /// Get the size of characters in bytes.
        /// </summary>
        public uint CharSize
        {
            get { return (uint)((m_bPrintAsciiStrings) ? sizeof(Byte) : sizeof(Char)); }
        }

        protected List<string> GetSortedKeys()
        {
            List<string> listKeys = new List<string>(m_hashStrings.Keys);
            listKeys.Sort(String.CompareOrdinal);
            return listKeys;
        }

        protected Dictionary<string, StringEntry> HashStrings
        {
            get { return m_hashStrings; }
        }

        protected uint m_uTotalMapSize = 0;
        private Dictionary<string, StringEntry> m_hashStrings = new Dictionary<string, StringEntry>();
        private bool m_bPrintAsciiStrings = false;
    };
}
