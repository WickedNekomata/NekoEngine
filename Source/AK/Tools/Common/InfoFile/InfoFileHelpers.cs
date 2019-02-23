using System;
using System.Collections.Generic;
using System.Text;
using System.Xml;
using System.Xml.Schema;

namespace AK.Wwise.InfoFile
{
    class InfoFileHelpers
    {
        const string INFO_SCHEMA_VERSION = "11";

        // Display any validation errors.
        private static void ValidationCallBack(object sender, ValidationEventArgs e)
        {
            throw new Exception("\n\nInvalid XML format. \n\n(" + e.Message + ")");
        }

        /// <summary>
        /// Load an soundbanks info file. Its data is returned in a SoundBanksInfo class.
        /// </summary>
        /// <param name="file">INFO file to be loaded</param>
        static internal AK.Wwise.InfoFile.SoundBanksInfo LoadInfoFile(string in_file)
        {
            XmlReaderSettings readerSettings = new XmlReaderSettings();
            readerSettings.ValidationType = ValidationType.Schema;
            string szSchemaFolder = System.IO.Path.GetDirectoryName(System.Windows.Forms.Application.ExecutablePath);
            string szSchemaFile = System.IO.Path.Combine(szSchemaFolder, @"..\..\..\..\Data\Schemas\SoundbanksInfo.xsd");
            readerSettings.Schemas.Add("", szSchemaFile);
            readerSettings.ValidationType = ValidationType.Schema;
            readerSettings.ValidationEventHandler += new ValidationEventHandler(ValidationCallBack);
            readerSettings.Schemas.Compile();

            //Check the schema version before loading the entire file.
            using (XmlReader reader = XmlReader.Create(in_file, readerSettings))
            {
                // Verify the schema version.
                reader.MoveToContent();
                if (reader.GetAttribute("SchemaVersion") != INFO_SCHEMA_VERSION)
                {
                    throw new Exception("Wrong Info file schema version. Please check the version of Wwise used to generate your SoundBanks.");
                }

                // This is the actual xsd validation for the entire file.
                while (reader.Read()) {}
            }


            AK.Wwise.InfoFile.SoundBanksInfo data = AK.Wwise.InfoFile.SoundBanksInfo.LoadFromFile(in_file);
            
            return data;
        }
    }

    public partial class FileDescriptorType : IEquatable<FileDescriptorType>
    {
        public virtual bool Equals(  FileDescriptorType obj )
        { 
            return this.Id == obj.Id && this.Language == obj.Language; 
        }
        public override int GetHashCode()
        {
            return Tuple.Create(Id, Language).GetHashCode();
        }
    }
}
