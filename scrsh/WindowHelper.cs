using System;
using System.Runtime.InteropServices;

namespace ScrSh
{
	/// <summary>
	/// Helper that aggregates PInvoke functions.
	/// </summary>
	public class WindowHelper
	{
		[DllImport("user32.dll")]
		private static extern IntPtr GetForegroundWindow();

		[DllImport("user32.dll", SetLastError = true)]
		private static extern bool GetWindowRect(IntPtr hWnd, out RECT lpRect);

		[StructLayout(LayoutKind.Sequential)]
		public struct RECT
		{
			public int Left;
			public int Top;
			public int Right;
			public int Bottom;
		}

		/// <summary>
		/// Retrieves active window coordinates.
		/// </summary>
		/// <returns>Region containing active window coordinates.</returns>
		public static Region GetActiveWindowRegion()
		{
			IntPtr handle = GetForegroundWindow();
			RECT rect;
			GetWindowRect(handle, out rect);
			return new Region(rect.Left, rect.Top, rect.Right - rect.Left, rect.Bottom - rect.Top);
		}
	}
}