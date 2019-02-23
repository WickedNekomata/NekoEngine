using System;
using System.Collections.Generic;
using System.Text;

namespace FilePackager.Packaging
{
    /// <summary>
    /// Header: Header definition of the file package.
    /// Handles only the general header information.
    /// After all elements of the header are created, their cumulative size
    /// should be computed and set explicitly here through HeaderSize().
    /// </summary>
    internal class Header
    {
        readonly uint AK_FILE_PACKAGE_VERSION = 1;

        public Header()
        {
        }
        /// <summary>
        /// Set header size.
        /// This is the value that gets written in the file package, and it should
        /// exclude the chunk definition (SizeOfChunkDefinition).
        /// Note: excludes header chunk definition, that is, 'AKPK' and header size.
        /// </summary>
        /// <param name="in_uHeaderSize">Header size (in bytes)</param>
        public uint HeaderSize
        {
            set { m_uHeaderSize = value - SizeOfChunkDefinition; }
        }

        /// <summary>
        /// Write header to file.
        /// </summary>
        /// <param name="in_writer">Binary writer.</param>
        public void Write(FilePackageWriter in_writer)
        {
            // Header ID.
            in_writer.WriteAscii("AKPK");

            // Header size. 
            in_writer.Write((uint)m_uHeaderSize);

            // Version.
            in_writer.Write((uint)AK_FILE_PACKAGE_VERSION);
        }

        /// <summary>
        /// Get the total size of the header chunk on disk.
        /// </summary>
        public static uint SizeOnDisk
        {
            get { return SIZE_OF_HEADER_CHUNK_DEF + sizeof(uint); }
        }
        /// <summary>
        /// Get the size of the chunk definition ('AKPK' and storage of header size).
        /// </summary>
        static public uint SizeOfChunkDefinition
        {
            get { return SIZE_OF_HEADER_CHUNK_DEF; }
        }
        private const uint SIZE_OF_HEADER_CHUNK_DEF = 4 + sizeof(uint);
        private uint m_uHeaderSize = 0;
    };
}
