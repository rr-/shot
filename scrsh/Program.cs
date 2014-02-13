using System;
using System.Drawing;
using System.Drawing.Imaging;
using System.IO;
using System.Linq;
using System.Windows.Forms;
using System.Reflection;

namespace ScrSh
{
	internal sealed class Program
	{
		/// <summary>
		/// Program version.
		/// </summary>
		public static string Version
		{
			get
			{
				return "2014-02-13";
			}
		}

		/// <summary>
		/// Main program function.
		/// </summary>
		[STAThread]
		private static void Main()
		{
			//parse command line arguments
			ProgramOptions programOptions;
			try
			{
				string[] args = Environment.GetCommandLineArgs();
				programOptions = ProgramOptionParser.Parse(args);
			}
			catch (Exception ex)
			{
				Console.Error.WriteLine(ex.Message);
				PrintUsage(Console.Error);
				Exit(ExitStatus.Failure);
				return;
			}

			try
			{
				if (programOptions.Path == null)
					programOptions.GetPathFromSaveDialog = true;

				if (programOptions.Region == null)
					programOptions.GetRegionFromGui = true;

				//add shift values
				if (programOptions.Shift != null)
				{
					if (programOptions.Region == null)
						throw new InvalidOperationException("It makes no sense to shift region, if it's going to be chosen from GUI");

					programOptions.Region.Left += programOptions.Shift.Left;
					programOptions.Region.Top += programOptions.Shift.Top;
					programOptions.Region.Width += programOptions.Shift.Width;
					programOptions.Region.Height += programOptions.Shift.Height;
				}

				//choose region using gui if neccessary
				if (programOptions.GetRegionFromGui)
					programOptions.Region = DoGetRegionFromGui(programOptions.Region);

				if (programOptions.Region == null ||
					programOptions.Region.Width <= 0 ||
					programOptions.Region.Height <= 0)
				{
					throw new InvalidOperationException("Specified region is empty");
				}

				Console.Out.WriteLine(
					"{0},{1};{2}x{3}",
					programOptions.Region.Left,
					programOptions.Region.Top,
					programOptions.Region.Width,
					programOptions.Region.Height);

				//make screenshot itself
				Bitmap image = GetScreenshot(programOptions.Region);

				//ask for filename if neccessary
				if (programOptions.GetPathFromSaveDialog)
					programOptions.Path = DoGetPathFromSaveDialog(programOptions.Path);

				if (programOptions.Path == null)
					throw new InvalidOperationException("Unspecified path");

				SaveScreenshot(image, programOptions);

				Exit(ExitStatus.Success);
			}
			catch (Exception ex)
			{
				Console.Error.WriteLine(ex.Message);
				Exit(ExitStatus.Failure);
			}
		}

		/// <summary>
		/// Prints program usage from embedded resources to specified stream.
		/// </summary>
		/// <param name="writer">Stream to print program usage to.</param>
		public static void PrintUsage(TextWriter writer)
		{
			using (Stream stream = Assembly.GetExecutingAssembly().GetManifestResourceStream("help.txt"))
			{
				if (stream == null)
					throw new NullReferenceException("Cannot read help file from resources");

				using (StreamReader reader = new StreamReader(stream))
				{
					string inputLine;
					while ((inputLine = reader.ReadLine()) != null)
					{
						string outputLine = inputLine;
						outputLine = outputLine.Replace("{program}", Path.GetFileName(Environment.GetCommandLineArgs()[0]));
						writer.WriteLine(outputLine);
					}
				}
			}
		}

		/// <summary>
		/// Exits the program with correct exit code.
		/// </summary>
		/// <param name="exitStatus">Used to derive exit code to exit with.</param>
		public static void Exit(ExitStatus exitStatus)
		{
			int exitCode;

			switch (exitStatus)
			{
				case ExitStatus.Success:
					exitCode = 0;
					break;

				default:
					exitCode = 1;
					break;
			}

			Environment.Exit(exitCode);
		}

		/// <summary>
		/// Saves specified screenshot using program options.
		/// </summary>
		/// <param name="image">Image to save.</param>
		/// <param name="programOptions">Program options to get path and save options from.</param>
		private static void SaveScreenshot(Bitmap image, ProgramOptions programOptions)
		{
			string format = Path.GetExtension(programOptions.Path);
			if (format == null)
				throw new FormatException("Unrecognized image format");
			format = format.ToLower();

			string dir = Path.GetDirectoryName(programOptions.Path);
			if (dir != null && !Directory.Exists(dir))
				Directory.CreateDirectory(dir);

			ImageCodecInfo encoder;
			EncoderParameters encoderParameters;

			switch (format)
			{
				case ".jpg":
					int quality = 66;
					if (programOptions.CompressionLevel.HasValue)
						quality = 100 - Math.Max(0, Math.Min(100, programOptions.CompressionLevel.Value));
					encoder = ImageCodecInfo.GetImageDecoders().Single(codec => codec.FormatID == ImageFormat.Jpeg.Guid);
					encoderParameters = new EncoderParameters(1);
					encoderParameters.Param[0] = new EncoderParameter(Encoder.Quality, quality);
					break;

				case ".png":
					encoder = ImageCodecInfo.GetImageDecoders().Single(codec => codec.FormatID == ImageFormat.Png.Guid);
					encoderParameters = null;
					break;

				default:
					throw new FormatException(string.Format("Unrecognized image format: {0}", format));
			}

			if (encoder == null)
				throw new NotSupportedException("Failed to load image encoder");

			image.Save(programOptions.Path, encoder, encoderParameters);
		}

		/// <summary>
		/// Retrieves region to take screenshot of, using graphical user interface.
		/// </summary>
		/// <param name="region">Region to initialize graphical user interface with. If null, default one will be used.</param>
		/// <returns>New region.</returns>
		private static Region DoGetRegionFromGui(Region region)
		{
			Application.EnableVisualStyles();
			Application.SetCompatibleTextRenderingDefault(false);
			RegionSelectionForm form = new RegionSelectionForm();
			form.Show();

			if (region == null)
			{
				const int width = 640;
				const int height = 480;

				region = new Region
				{
					Width = width,
					Height = height,
					Left = (Screen.PrimaryScreen.Bounds.Width - width) / 2,
					Top = (Screen.PrimaryScreen.Bounds.Height - height) / 2,
				};
			}

			form.SelectedRegion = region;
			Application.Run(form);

			if (!form.Success)
				throw new OperationCanceledException("Cancelled by user");

			return form.SelectedRegion;
		}

		/// <summary>
		/// Retrieves path to save screenshot to, using graphical user interface.
		/// </summary>
		/// <param name="suppliedPath">Default path to initialize save dialog with.</param>
		/// <returns>New save path.</returns>
		private static string DoGetPathFromSaveDialog(string suppliedPath)
		{
			SaveFileDialog dialog = new SaveFileDialog
			{
				Title = "Save file as...",
				Filter = "JPEG files (*.jpg)|*.jpg|PNG files (*.png)|*.png|All files (*.*)|*.*",
				RestoreDirectory = false
			};
			if (suppliedPath != null)
			{
				string dir = Path.GetDirectoryName(suppliedPath);
				if (dir != null && Directory.Exists(dir))
					dialog.InitialDirectory = Path.GetDirectoryName(suppliedPath);

				dialog.FileName = Path.GetFileName(suppliedPath);
			}

			if (dialog.ShowDialog() != DialogResult.OK)
				throw new OperationCanceledException("Cancelled by user");

			return dialog.FileName;
		}

		/// <summary>
		/// Retrieves screenshot of specified region.
		/// </summary>
		/// <param name="region">Region to make screenshot of.</param>
		/// <returns>Bitmap containing screenshot.</returns>
		private static Bitmap GetScreenshot(Region region)
		{
			var image = new Bitmap(region.Width, region.Height, PixelFormat.Format24bppRgb);
			Graphics g = Graphics.FromImage(image);
			g.CopyFromScreen(region.Left, region.Top, 0, 0, new Size(region.Width, region.Height), CopyPixelOperation.SourceCopy);
			return image;
		}
	}
}
