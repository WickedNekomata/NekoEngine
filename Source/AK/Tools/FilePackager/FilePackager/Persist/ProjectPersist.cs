using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using FilePackager.Models;
using System.Xml.Linq;
using FilePackager.Base;

namespace FilePackager.Persist
{
	static public class ProjectPersist
	{
		public static Project Load(string file)
		{
            Migration.Migrate(file);

			XDocument doc = XDocument.Load(file);

            Project project = new Project();
            project.InfoFilePath = doc.Root.Attribute("InfoFilePath").Value;
            project.OutputDirectory = doc.Root.Attribute("OutputDirectory").Value;

            LoadManualPackaging(doc.Root, project.ManualPackagingInfo);

            project.FilePath = file;

			return project;
		}

		private static void LoadManualPackaging(XElement xParent, ManualPackagingInfo manualPackagingInfo)
		{
			XElement xElement = xParent.Element("ManualPackagingInfo");
			
            manualPackagingInfo.BlockSize = uint.Parse(xElement.Attribute("BlockSize").Value);
            manualPackagingInfo.UnassignedBanksPackageId = new Guid(xElement.Attribute("UnassignedBanksPackageId").Value);
            manualPackagingInfo.UnassignedStreamsPackageId = new Guid(xElement.Attribute("UnassignedStreamsPackageId").Value);
            manualPackagingInfo.UnassignedExternalsPackageId = new Guid(xElement.Attribute("UnassignedExternalsPackageId").Value);
            manualPackagingInfo.UnassignedLooseMediaPackageId = new Guid(xElement.Attribute("UnassignedLooseMediaPackageId").Value);

            // Load packages
            XElement xPackages = xElement.Element("Packages");
            manualPackagingInfo.Packages.AddRange(
                xPackages.Elements().Select(p => new Package(new Guid(p.Attribute("Id").Value), 
                    p.Element("Items").Elements().Select(i => 
                        new PackageContentItem(UInt64.Parse(i.Attribute("Id").Value), i.Attribute("Language").Value, i.Attribute("FileName").Value)
			            {
                            InclusionMode = (InclusionMode)Enum.Parse(typeof(InclusionMode), i.Attribute("InclusionMode").Value) 
			            } ),
                    p.Element("LayoutItems").Elements().Select(i => 
                        new ContentItemBase(UInt64.Parse(i.Attribute("Id").Value), i.Attribute("Language").Value, i.Attribute("FileName").Value))
                    )
			    {
				    Name = p.Attribute("Name").Value
			    }));

            // Load DefaultLanguagePackageIds
            XElement xDefaultLanguagePackageIds = xElement.Element("DefaultLanguagePackageIds");
            manualPackagingInfo.DefaultLanguagePackageIds.AddRange(
                xDefaultLanguagePackageIds.Elements().Select(e => new LanguagePackageIds(e.Attribute("Language").Value)
                {
                    BankPackageId = new Guid(e.Attribute("BankPackageId").Value),
                    StreamPackageId = new Guid(e.Attribute("StreamPackageId").Value),
                    LooseMediaPackageId = new Guid(e.Attribute("LooseMediaPackageId").Value),
                }));
        }

		public static void Save(Project project)
		{
            // Create new document
			XDocument doc = new XDocument(
				new XDeclaration("1.0", "utf-8", "yes"),
				new XElement("Project",

                    new XAttribute("Version", Migration.CurrentVersion.ToString()),

                    // Save project
					new XAttribute("InfoFilePath", project.InfoFilePath),
                    new XAttribute("OutputDirectory", project.OutputDirectory),
					SaveManualPackaging(project.ManualPackagingInfo)));

			doc.Save(project.FilePath);
		}

		private static XElement SaveManualPackaging(ManualPackagingInfo manualPackagingInfo)
		{
			return new XElement("ManualPackagingInfo",
				
                new XAttribute("BlockSize", manualPackagingInfo.BlockSize.ToString()),
                new XAttribute("UnassignedBanksPackageId", manualPackagingInfo.UnassignedBanksPackageId.ToString()),
                new XAttribute("UnassignedStreamsPackageId", manualPackagingInfo.UnassignedStreamsPackageId.ToString()),
                new XAttribute("UnassignedExternalsPackageId", manualPackagingInfo.UnassignedExternalsPackageId.ToString()),
                new XAttribute("UnassignedLooseMediaPackageId", manualPackagingInfo.UnassignedLooseMediaPackageId.ToString()),

                // Save packages
                new XElement("Packages",
                    manualPackagingInfo.Packages.Select(p => new XElement("Package",
				        new XAttribute("Id", p.Id),
				        new XAttribute("Name", p.Name),
                        
                        new XElement("Items",
				            p.Items.Select(pci => new XElement("PackageContentItem",
					            new XAttribute("Id", pci.Id.ToString()),
                                new XAttribute("Language", pci.Language),
                                new XAttribute("FileName", pci.FileName),
                                new XAttribute("InclusionMode", pci.InclusionMode.ToString())
                                ))),

                        new XElement("LayoutItems",
				            p.LayoutItems.Select(pci => new XElement("ContentItemBase",
					            new XAttribute("Id", pci.Id.ToString()),
                                new XAttribute("Language", pci.Language),
                                new XAttribute("FileName", pci.FileName))))
                                ))),

                // Save DefaultLanguagePackageIds
                new XElement("DefaultLanguagePackageIds",
                    manualPackagingInfo.DefaultLanguagePackageIds.Select( dlpi => 
                        new XElement("LanguagePackageIds",
                            new XAttribute("Language", dlpi.Language),
                            new XAttribute("BankPackageId", dlpi.BankPackageId.ToString()),
                            new XAttribute("StreamPackageId", dlpi.StreamPackageId.ToString()),
                            new XAttribute("LooseMediaPackageId", dlpi.LooseMediaPackageId.ToString())
                            )))
                    );
		}
	}
}
