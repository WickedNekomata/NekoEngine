using System;
using System.Collections.Generic;
using System.Text;
using System.IO;

namespace FilePackager.Packaging
{
    /// <summary>
    /// Implements a binary writer.
    /// Handles byte swapping (for Big-Endian platforms)
    /// and writing of null-terminated strings.
    /// </summary>
    internal class FilePackageWriter
    {
        internal enum Endianness
        {
            LittleEndian,
            BigEndian
        };

        public FilePackageWriter(FileStream in_file, Endianness in_eEndianness)
        {
            m_writer = new BinaryWriter(in_file);
            m_bDoByteSwap = BitConverter.IsLittleEndian && (in_eEndianness == Endianness.BigEndian) ||
                            !BitConverter.IsLittleEndian && (in_eEndianness == Endianness.LittleEndian);
        }

        public void Close()
        {
            m_writer.Close();
        }

        public ulong Position
        {
            get { return (ulong)m_writer.BaseStream.Position; }
        }

        /// <summary>
        /// Write a string as ASCII characters.
        /// </summary>
        /// <param name="in_string">String to be written.</param>
        public void WriteAscii(string in_string)
        {
            m_writer.Write(Encoding.ASCII.GetBytes(in_string));
        }

        public void Write(UInt16 in_uValue)
        {
            if (m_bDoByteSwap)
            {
                byte[] ar = BitConverter.GetBytes(in_uValue);
                Array.Reverse(ar);
                m_writer.Write(ar);
            }
            else
                m_writer.Write(in_uValue);
        }

        public void Write(UInt32 in_uValue)
        {
            if (m_bDoByteSwap)
            {
                byte[] ar = BitConverter.GetBytes(in_uValue);
                Array.Reverse(ar);
                m_writer.Write(ar);
            }
            else
                m_writer.Write(in_uValue);
        }

        public void Write(UInt64 in_uValue)
        {
            if (m_bDoByteSwap)
            {
                byte[] ar = BitConverter.GetBytes(in_uValue);
                Array.Reverse(ar);
                m_writer.Write(ar);
            }
            else
                m_writer.Write(in_uValue);
        }

        /// <summary>
        /// Write an unicode or an ansi string.
        /// </summary>
        /// <param name="in_string">String to be written.</param>
        /// <param name="in_bAnsi">Writes an ANSI string if true.</param>
        public void Write(string in_string, bool in_bAnsi)
        {
            if (!in_bAnsi)
                Write(in_string);   // Default implementation.
            else
            {
                WriteAscii(in_string);
                m_writer.Write((byte)0);  // NULL character.
            }
        }
        
        /// <summary>
        /// Write a unicode string.
        /// Overrides BinaryWriter.Write(string).
        /// </summary>
        /// <param name="in_string">String to be written.</param>
        public void Write(string in_string)
        {
            if (!m_bDoByteSwap)
                m_writer.Write(Encoding.Convert(Encoding.ASCII, Encoding.Unicode, Encoding.ASCII.GetBytes(in_string)));
            else
                m_writer.Write(Encoding.Convert(Encoding.ASCII, Encoding.BigEndianUnicode, Encoding.ASCII.GetBytes(in_string)));
            m_writer.Write((ushort)0);  // NULL character.
        }

        public void Write(byte[] in_bytes)
        {
            m_writer.Write(in_bytes);
        }

        private BinaryWriter m_writer;
        private bool m_bDoByteSwap;
    };
}
