using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using FilePackager.Base;

namespace FilePackager.Models
{
    [Flags]
    public enum InclusionMode
    {
        [ResourcesDescription("InclusionModeAllFiles", "FilePackager")]
        BankAndStreamsAndLooseMedia = (Bank | Streams | LooseMedia),

        [ResourcesDescription("InclusionModeBank", "FilePackager")]
        Bank = 1 << 0,

        [ResourcesDescription("InclusionModeStreams", "FilePackager")]
        Streams = 1 << 1,

        [ResourcesDescription("InclusionModeLooseMedia", "FilePackager")]
        LooseMedia = 1 << 2,

        [ResourcesDescription("InclusionModeBankAndStreams", "FilePackager")]
        BankAndStreams = (Bank | Streams),

        [ResourcesDescription("InclusionModeBankAndLooseMedia", "FilePackager")]
        BankAndLooseMedia = (Bank | LooseMedia),

        [ResourcesDescription("InclusionModeStreamsAndLooseMedia", "FilePackager")]
        StreamsAndLooseMedia = (Streams | LooseMedia),

        [ResourcesDescription("InclusionModeExternal", "FilePackager")]
        [EnumValueIsVisible(false)]
        External = 1 << 4,

        [ResourcesDescription("InclusionModeInvalid", "FilePackager")]
        [EnumValueIsVisible(false)]
        Invalid = 0,
    }
    
    public class PackageContentItem : ContentItemBase
	{
        private InclusionMode _inclusionMode = InclusionMode.BankAndStreamsAndLooseMedia;

        public PackageContentItem( UInt64 id, string language, string fileName )
            :base(id, language,fileName)
		{
		}

        public InclusionMode InclusionMode
		{
            get { return _inclusionMode; }
            set { SetValue(ref _inclusionMode, value, "InclusionMode"); }
		}
    }
}
