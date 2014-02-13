using System;
using System.Collections.Generic;
using System.IO;
using System.Text.RegularExpressions;
using System.Windows.Forms;

namespace ScrSh
{
	public class ProgramOptionParser
	{
		private const string RegionRegex = "^([-+]?\\d+)[:,]([-+]?\\d+)[;,]([-+]?\\d+)[x:,]([-+]?\\d+)$";

		/// <summary>
		/// Parses human-readable arguments into instance of ProgramOptions.
		/// </summary>
		/// <param name="args">List of arguments.</param>
		/// <returns>Parsed program options.</returns>
		public static ProgramOptions Parse(IEnumerable<string> args)
		{
			var programOptions = new ProgramOptions();
			ArgumentQueue argsQueue = new ArgumentQueue(args);
			argsQueue.Dequeue(); //path to program

			while (argsQueue.Count > 0)
			{
				string arg = argsQueue.Dequeue().ToLower();
				ParseArgument(arg, argsQueue, programOptions);
			}

			return programOptions;
		}

		/// <summary>
		/// Parses one or more arguments.
		/// </summary>
		/// <param name="arg">Current argument.</param>
		/// <param name="argsQueue">Queue containing the rest of arguments.</param>
		/// <param name="programOptions">Program options to decorate.</param>
		private static void ParseArgument(
			string arg,
			ArgumentQueue argsQueue,
			ProgramOptions programOptions)
		{
			switch (arg)
			{
				case "-h":
				case "--help":
					Program.PrintUsage(Console.Out);
					Program.Exit(ExitStatus.Success);
					break;

				case "-v":
				case "--version":
					Console.Out.WriteLine(Program.Version);
					Program.Exit(ExitStatus.Success);
					break;

				case "-p":
				case "--path":
					programOptions.Path = Path.GetFullPath(argsQueue.Dequeue());
					break;

				case "--save-dialog":
				case "--force-save-dialog":
					programOptions.GetPathFromSaveDialog = true;
					break;

				case "-g":
				case "--gui":
				case "--force-gui":
					programOptions.GetRegionFromGui = true;
					break;

				case "-c":
				case "--compression":
					programOptions.CompressionLevel = Convert.ToInt32(argsQueue.Dequeue());
					break;

				case "-r":
				case "--region":
					programOptions.Region = ParseRegionArg(argsQueue.Dequeue().ToLower());
					break;

				case "-s":
				case "--shift":
					Match match;
					arg = argsQueue.Dequeue().ToLower();
					if ((match = new Regex(RegionRegex).Match(arg)).Success)
					{
						programOptions.Shift = new Region
						{
							Left = Convert.ToInt32(match.Groups[1].Value),
							Top = Convert.ToInt32(match.Groups[2].Value),
							Width = Convert.ToInt32(match.Groups[3].Value),
							Height = Convert.ToInt32(match.Groups[4].Value),
						};
					}
					else
						throw new FormatException(string.Format("Invalid shift: {0}", arg));

					break;

				default:
					throw new ArgumentException(string.Format("Unknown argument: {0}", arg));
			}
		}

		/// <summary>
		/// Parses argument representing region string.
		/// </summary>
		/// <param name="arg">Argument that contains human-readable region string.</param>
		/// <returns>Parsed instance of Region.</returns>
		private static Region ParseRegionArg(string arg)
		{
			switch (arg)
			{
				case "all-monitors":
					Region r = new Region(Screen.PrimaryScreen.Bounds);
					foreach (Screen screen in Screen.AllScreens)
					{
						r.Left = Math.Min(r.Left, screen.Bounds.Left);
						r.Top = Math.Min(r.Top, screen.Bounds.Top);
						r.Right = Math.Max(r.Right, screen.Bounds.Right);
						r.Bottom = Math.Max(r.Bottom, screen.Bounds.Bottom);
					}
					return r;

				case "primary-monitor":
					return new Region(Screen.PrimaryScreen.Bounds);

				case "active-monitor":
					foreach (Screen screen in Screen.AllScreens)
						if (screen.Bounds.Contains(Cursor.Position))
							return new Region(screen.Bounds);
					throw new InvalidOperationException("Cursor wasn't found in any monitor");

				case "active-window":
					return WindowHelper.GetActiveWindowRegion();

				default:
					Match match;
					if ((match = new Regex("^monitor[-,](\\d+)$").Match(arg)).Success)
					{
						int monitorNum = Convert.ToInt32(match.Groups[1].Value);
						if (monitorNum < 0 || monitorNum >= Screen.AllScreens.Length)
						{
							throw new ArgumentException(string.Format(
								"Monitor {0} out of range (valid monitors: 1..{1})",
								monitorNum,
								Screen.AllScreens.Length));
						}

						return new Region(Screen.AllScreens[monitorNum - 1].Bounds);
					}

					if ((match = new Regex(RegionRegex).Match(arg)).Success)
					{
						return new Region
						{
							Left = Convert.ToInt32(match.Groups[1].Value),
							Top = Convert.ToInt32(match.Groups[2].Value),
							Width = Convert.ToInt32(match.Groups[3].Value),
							Height = Convert.ToInt32(match.Groups[4].Value),
						};
					}

					throw new FormatException(string.Format("Invalid region: {0}", arg));
			}
		}
	}
}