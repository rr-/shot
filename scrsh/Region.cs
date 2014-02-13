using System.Drawing;

namespace ScrSh
{
	public class Region
	{
		public int Left { get; set; }
		public int Top { get; set; }
		public int Width { get; set; }
		public int Height { get; set; }

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
}