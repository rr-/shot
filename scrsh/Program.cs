using System;
using System.Collections.Generic;
using System.Drawing;
using System.Drawing.Imaging;
using System.IO;
using System.Runtime.InteropServices;
using System.Text.RegularExpressions;
using System.Windows.Forms;
using System.Reflection;

namespace ScrSh
{
	internal sealed class Program
	{
		private static readonly Int32 EXIT_SUCCESS = 0;
		private static readonly Int32 EXIT_FAILURE = 1;
		[DllImport("user32.dll")] static extern IntPtr GetForegroundWindow();
		[DllImport("user32.dll", SetLastError = true)] static extern bool GetWindowRect(IntPtr hWnd, out RECT lpRect);
		[StructLayout(LayoutKind.Sequential)]
		public struct RECT
		{
			public int Left;
			public int Top;
			public int Right;
			public int Bottom;
		}

		private class Region
		{
			public int Left = 0;
			public int Top = 0;
			public int Width = 0;
			public int Height = 0;

			public int Right
			{
				get { return Left + Width; }
				set { Width = value - Left; }
			}
			public int Bottom
			{
				get { return Top + Height; }
				set { Height = value - Top; }
			}

			public Region()
			{
			}

			public Region(Rectangle rect)
			{
				Left = rect.Left;
				Top = rect.Top;
				Width = rect.Width;
				Height = rect.Height;
			}

			public Region(int left, int top, int width, int height)
			{
				Left = left;
				Top = top;
				Width = width;
				Height = height;
			}
		}

		private static void printUsage(TextWriter writer)
		{
			using (Stream stream = Assembly.GetExecutingAssembly().GetManifestResourceStream("help.txt"))
			{
				using (StreamReader reader = new StreamReader(stream))
				{
					String inputLine = null;
					while ((inputLine = reader.ReadLine()) != null)
					{
						String outputLine = inputLine;
						outputLine = outputLine.Replace("{program}", System.IO.Path.GetFileName(Environment.GetCommandLineArgs()[0]));
						writer.WriteLine(outputLine);
					}
				}
			}

		}


		[STAThread]
		private static void Main(string[] _args)
		{
			Match match;
			Region region = null;
			Region shift = null;

			bool createGUI = false;
			bool createSaveDialog = false;
			String path = null;
			String format = null;

			//parse command line arguments
			String[] args = Environment.GetCommandLineArgs();
			try
			{
				for (int i = 1; i < args.Length; i ++)
				{
					String arg = args[i].ToLower();
					switch (arg)
					{
						case "-h":
						case "--help":
							Program.printUsage(Console.Out);
							Environment.Exit(0);
							break;

						case "-v":
						case "--version":
							Console.Out.WriteLine("2013-02-18");
							Environment.Exit(0);
							break;

						case "-f":
						case "--format":
							i ++;
							format = args[i].ToLower();
							break;

						case "-p":
						case "--path":
							i ++;
							try
							{
								path = System.IO.Path.GetFullPath(args[i]);
							}
							catch (Exception e)
							{
								Console.Error.WriteLine(String.Format("Invalid path \"{0}\" ({1})", path, e.Message));
								Environment.Exit(EXIT_FAILURE);
								return;
							}
							break;

						case "--save-dialog":
						case "--force-save-dialog":
							createSaveDialog = true;
							break;

						case "-g":
						case "--gui":
						case "--force-gui":
							createGUI = true;
							break;

						case "-r":
						case "--region":
							i ++;
							arg = args[i].ToLower();
							if (arg == "all-monitors")
							{
								region = new Region(Screen.PrimaryScreen.Bounds);
								foreach (Screen screen in Screen.AllScreens)
								{
									region.Left = Math.Min(region.Left, screen.Bounds.Left);
									region.Top = Math.Min(region.Top, screen.Bounds.Top);
									region.Right = Math.Max(region.Right, screen.Bounds.Right);
									region.Bottom = Math.Max(region.Bottom, screen.Bounds.Bottom);
								}
							}
							else if (arg == "primary-monitor")
							{
								region = new Region(Screen.PrimaryScreen.Bounds);
							}
							else if (arg == "active-monitor")
							{
								foreach (Screen screen in Screen.AllScreens)
								{
									if (screen.Bounds.Contains(Cursor.Position))
									{
										region = new Region(screen.Bounds);
									}
								}
								if (region == null)
								{
									Console.Error.WriteLine("Invalid cursor position (?)");
									Environment.Exit(EXIT_FAILURE);
									return;
								}
							}
							else if (arg == "active-window")
							{
								IntPtr handle = GetForegroundWindow();
								RECT rect;
								GetWindowRect(handle, out rect);
								region = new Region(rect.Left, rect.Top, rect.Right - rect.Left, rect.Bottom - rect.Top);
							}
							else if ((match = new Regex("^monitor[-,](\\d+)$").Match(arg)).Success)
							{
								int monitorNum = Convert.ToInt32(match.Groups[1].Value);
								try
								{
									region = new Region(Screen.AllScreens[monitorNum - 1].Bounds);
								}
								catch (IndexOutOfRangeException)
								{
									Console.Error.WriteLine(String.Format("Monitor{0} out of range (valid monitors: 1..{1})", monitorNum, Screen.AllScreens.Length));
									Environment.Exit(EXIT_FAILURE);
									return;
								}
							}
							else if ((match = new Regex("^(-?\\d+)[:,](-?\\d+)[x;,](\\d+)[:,](\\d+)$").Match(arg)).Success)
							{
								int x = Convert.ToInt32(match.Groups[1].Value);
								int y = Convert.ToInt32(match.Groups[2].Value);
								int w = Convert.ToInt32(match.Groups[3].Value);
								int h = Convert.ToInt32(match.Groups[4].Value);
								region = new Region(x, y, w, h);
							}
							else
							{
								throw new FormatException("Invalid region: " + arg);
							}
							break;

						case "-s":
						case "--shift":
							i ++;
							arg = args[i].ToLower();
							if ((match = new Regex("^([-+]?\\d+)[:,]([-+]?\\d+)[x;,]([-+]?\\d+)[:,]([-+]?\\d+)$").Match(arg)).Success)
							{
								int x1 = Convert.ToInt32(match.Groups[1].Value);
								int y1 = Convert.ToInt32(match.Groups[2].Value);
								int x2 = Convert.ToInt32(match.Groups[3].Value);
								int y2 = Convert.ToInt32(match.Groups[4].Value);
								shift = new Region(x1, y1, x2, y2);
							}
							else
							{
								throw new FormatException("Invalid shift region: " + arg);
							}
							break;
					}
				}
			}
			catch (IndexOutOfRangeException)
			{
				Console.Error.WriteLine("Expected command line argument was not found");
				printUsage(Console.Error);
				Environment.Exit(EXIT_FAILURE);
				return;
			}
			catch (FormatException)
			{
				Console.Error.WriteLine("Specified argument is invalid");
				printUsage(Console.Error);
				Environment.Exit(EXIT_FAILURE);
				return;
			}

			if (path == null)
			{
				createSaveDialog = true;
			}
			if (region == null)
			{
				region = new Region();
				region.Width = 640;
				region.Height = 480;
				region.Left = (Screen.PrimaryScreen.Bounds.Width - region.Width) / 2;
				region.Top = (Screen.PrimaryScreen.Bounds.Height - region.Height) / 2;
				createGUI = true;
			}

			//add shift values
			if (shift != null)
			{
				region.Left += shift.Left;
				region.Top += shift.Top;
				region.Width += shift.Width;
				region.Height += shift.Height;
			}

			//choose region using gui if neccessary
			if (createGUI)
			{
				Application.EnableVisualStyles();
				Application.SetCompatibleTextRenderingDefault(false);
				MainForm form = new MainForm();
				form.Show();
				if (region != null)
				{
					form.Left = region.Left;
					form.Top = region.Top;
					form.Size = new Size(region.Width, region.Height);
				}
				Application.Run(form);
				if (!form.success)
				{
					Console.WriteLine("Cancelled by user");
					Environment.Exit(EXIT_FAILURE);
					return;
				}
				region = new Region(form.Left, form.Top, form.Width, form.Height);
			}

			if (region.Width == 0 || region.Height == 0)
			{
				Console.Error.WriteLine("Specified region is empty");
				Environment.Exit(EXIT_FAILURE);
				return;
			}

			//make screenshot itself
			Bitmap image = new Bitmap(region.Width, region.Height, PixelFormat.Format24bppRgb);
			Graphics g = Graphics.FromImage(image);
			g.CopyFromScreen(region.Left, region.Top, 0, 0, new Size(region.Width, region.Height), CopyPixelOperation.SourceCopy);

			//ask for filename if neccessary
			if (createSaveDialog)
			{
				SaveFileDialog dialog = new SaveFileDialog();
				dialog.Title = "Save file as...";
				dialog.Filter = "JPEG files (*.jpg)|*.jpg|PNG files (*.png)|*.png|All files (*.*)|*.*";
				dialog.RestoreDirectory = false;
				if (path != null)
				{
					String dir = System.IO.Path.GetDirectoryName(path);
					if (System.IO.Directory.Exists(dir))
					{
						dialog.InitialDirectory = System.IO.Path.GetDirectoryName(path);
					}
					dialog.FileName = System.IO.Path.GetFileName(path);
				}

				if (dialog.ShowDialog() == DialogResult.OK)
				{
					path = dialog.FileName;
				}
				else
				{
					Console.WriteLine("Cancelled by user");
					Environment.Exit(EXIT_FAILURE);
					return;
				}
			}
			if (createSaveDialog || format == null)
			{
				//overwrite format with extension
				format = System.IO.Path.GetExtension(path).Replace(".", "");
			}

			{
				String dir = System.IO.Path.GetDirectoryName(path);
				if (!System.IO.Directory.Exists(dir))
				{
					System.IO.Directory.CreateDirectory(dir);
				}
			}

			try
			{
				if ((match = new Regex("^jpg(:([0-9]+))?$").Match(format)).Success)
				{
					ImageCodecInfo jpgDecoder = null;
					foreach (ImageCodecInfo codec in ImageCodecInfo.GetImageDecoders())
					{
						if (codec.FormatID == ImageFormat.Jpeg.Guid)
						{
							jpgDecoder = codec;
						}
					}
					if (jpgDecoder == null)
					{
						Console.Error.WriteLine("Failed to load JPEG decoder");
						Environment.Exit(EXIT_FAILURE);
						return;
					}
					int compressionLevel = 66;
					if (match.Groups[2].Success)
					{
						compressionLevel = Convert.ToInt32(match.Groups[2].Value);
					}
					System.Drawing.Imaging.Encoder encoder = Encoder.Compression;
					EncoderParameters encoderParameters = new EncoderParameters(1);
					encoderParameters.Param[0] = new EncoderParameter(encoder, compressionLevel);
					image.Save(path, jpgDecoder, encoderParameters);
				}
				else if (format == "png")
				{
					image.Save(path, ImageFormat.Png);
				}
				else
				{
					Console.Error.WriteLine(String.Format("Unrecognized format: {0}", format));
					Environment.Exit(EXIT_FAILURE);
					return;
				}
			}
			catch (IOException e)
			{
				Console.Error.WriteLine(String.Format("Failed to write screenshot to \"{0}\" ({1})", path, e.Message));
				Environment.Exit(EXIT_FAILURE);
				return;
			}

			Environment.Exit(EXIT_SUCCESS);
		}
	}
}
