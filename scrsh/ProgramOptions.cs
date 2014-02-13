using System;

namespace ScrSh
{
	public class ProgramOptions
	{
		public Region Region { get; set; }
		public Region Shift { get; set; }

		public bool GetRegionFromGui { get; set; }
		public bool GetPathFromSaveDialog { get; set; }
		public String Path { get; set; }

		public int? CompressionLevel { get; set; }
	};
}