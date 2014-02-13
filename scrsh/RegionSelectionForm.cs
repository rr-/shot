using System;
using System.Drawing;
using System.Windows.Forms;

namespace ScrSh
{
	/// <summary>
	/// Gui selector for Region.
	/// </summary>
	public partial class RegionSelectionForm : Form
	{
		public const UInt16 WM_NCHITTEST = 0x0084;
		public const UInt16 HTCAPTION = 2,
			HTLEFT = 10,
			HTRIGHT = 11,
			HTTOP = 12,
			HTTOPLEFT = 13,
			HTTOPRIGHT = 14,
			HTBOTTOM = 15,
			HTBOTTOMLEFT = 16,
			HTBOTTOMRIGHT = 17;

		private const int GripSize = 16;
		private const int SlowMovementSpeed = 1;
		private const int NormalMovementSpeed = 16;

		/// <summary>
		/// Whether the GUI completed successfully, i.e. user hans't cancelled region selection
		/// </summary>
		public bool Success { get; protected set; }

		/// <summary>
		/// Region selected by user.
		/// </summary>
		public Region SelectedRegion
		{
			get { return new Region(Left, Top, Width, Height); }
			set
			{
				Left = value.Left;
				Top = value.Top;
				Size = new Size(value.Width, value.Height);
			}
		}

		public RegionSelectionForm()
		{
			InitializeComponent();
			SetStyle(ControlStyles.ResizeRedraw, true);
			Success = false;
		}

		/// <summary>
		/// Overrides default paint procedure.
		/// </summary>
		/// <param name="e">Arguments for paint procedure.</param>
		protected override void OnPaint(PaintEventArgs e)
		{
			//draw resize grip

			var rc = new Rectangle(
				ClientSize.Width - GripSize,
				ClientSize.Height - GripSize,
				GripSize,
				GripSize);

			ControlPaint.DrawSizeGrip(e.Graphics, BackColor, rc);
		}

		/// <summary>
		/// Overrides default window procedure.
		/// </summary>
		/// <param name="m">Message for window procedure.</param>
		protected override void WndProc(ref Message m)
		{
			if (m.Msg != WM_NCHITTEST)
			{
				base.WndProc(ref m);
				return;
			}

			//change mouse cursor on window border
			Point pos = new Point(m.LParam.ToInt32() & 0xffff, m.LParam.ToInt32() >> 16);
			pos = PointToClient(pos);
			int sectionHorizontal = 0;
			int sectionVertical = 0;

			UInt16[,] results =
			{
				{HTTOPLEFT, HTTOP, HTTOPRIGHT},
				{HTLEFT, HTCAPTION, HTRIGHT},
				{HTBOTTOMLEFT, HTBOTTOM, HTBOTTOMRIGHT}
			};

			if (pos.X < GripSize)
				sectionHorizontal = - 1;

			else if (pos.X >= ClientSize.Width - GripSize)
				sectionHorizontal = 1;

			if (pos.Y < GripSize)
				sectionVertical = -1;

			else if (pos.Y >= ClientSize.Height - GripSize)
				sectionVertical = 1;

			m.Result = (IntPtr) results[sectionVertical + 1, sectionHorizontal + 1];
		}

		public void RegionSelectionFormKeyDown(object sender, KeyEventArgs e)
		{
			switch (e.KeyCode)
			{
				case Keys.Escape:
					Close();
					break;

				case Keys.Enter:
					Success = true;
					Close();
					break;

				case Keys.Left:
				case Keys.Down:
				case Keys.Right:
				case Keys.Up:
					int delta = e.Control
						? SlowMovementSpeed
						: NormalMovementSpeed;

					if (e.KeyCode == Keys.Left || e.KeyCode == Keys.Up)
						delta *= -1;

					if (e.Shift)
					{
						if (e.KeyCode == Keys.Right || e.KeyCode == Keys.Left)
							Width += delta;
						else
							Height += delta;
					}
					else
					{
						if (e.KeyCode == Keys.Right || e.KeyCode == Keys.Left)
							Left += delta;
						else
							Top += delta;
					}
					break;
			}
		}
	}
}
