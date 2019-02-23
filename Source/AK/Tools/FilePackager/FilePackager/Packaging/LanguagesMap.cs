using System;
using System.Collections.Generic;
using System.Text;

namespace FilePackager.Packaging
{
    /// <summary>
    /// Language map: extends StringMap for languages. The constructor takes the
    /// hash of LanguageNames-LanguageIDs and fills the map.
    /// </summary>
    internal class LanguagesMap : StringMap
    {
        /// <summary>
        /// Constructor. Takes the hash of LanguageNames-LanguageIDs and fills the collection.
        /// </summary>
        /// <param name="in_mapLanguages">Hash of LanguageNames-LanguageIDs.</param>
        /// <param name="in_bPrintAsciiStrings">All strings are converted to asciiwhen written to disk if true. CharSize returns appropriate character size</param>
        public LanguagesMap(Dictionary<string, uint> in_mapLanguages, bool in_bPrintAsciiStrings)
            : base(in_bPrintAsciiStrings)
        {
            // Get data from language collection.
            foreach (string szLanguage in in_mapLanguages.Keys)
            {
                AddString(szLanguage, new StringEntry(in_mapLanguages[szLanguage]));
            }

            // Initial string offset is the position of the first string relative to the beginning of
            // the string map (including the string count, hence the "+sizeof uint").
            uint uStringOffset = (uint)HashStrings.Count * StringEntry.SizeOfEntryOnDisk + sizeof(uint);

            // Compute offset for each string.
            // Strings are sorted so that the run-time lookup can perform a binary search.
            List<string> sortedStrings = GetSortedKeys();
            foreach (string szLanguage in sortedStrings)
            {
                HashStrings[szLanguage].Offset = uStringOffset;
                uStringOffset += (uint)(szLanguage.Length + 1) * CharSize;
            }

            m_uTotalMapSize = uStringOffset;
        }
    };
}
