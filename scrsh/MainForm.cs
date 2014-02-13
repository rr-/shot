using System;
using System.Collections.Generic;
using System.Drawing;
using System.Windows.Forms;
using System.Runtime.InteropServices;

namespace ScrSh
{
	public partial class MainForm : Form
	{
		public static readonly UInt16 WM_NCHITTEST = 0x0084;
		public static Int16 HTCAPTION = 2,
			HTLEFT = 10,
			HTRIGHT = 11,
			HTTOP = 12,
			HTTOPLEFT = 13,
			HTTOPRIGHT = 14,
			HTBOTTOM = 15,
			HTBOTTOMLEFT = 16,
			HTBOTTOMRIGHT = 17;

		private const int gripSize = 16;
		private const int movementSpeed = 16;
		public bool success { get; protected set; }

		public MainForm()
		{
			InitializeComponent();
			SetStyle(System.Windows.Forms.ControlStyles.ResizeRedraw, true);
			success = false;
		}

		protected override void OnPaint(PaintEventArgs e)
		{
			Rectangle rc = new Rectangle(this.ClientSize.Width - gripSize, this.ClientSize.Height - gripSize, gripSize, gripSize);
			ControlPaint.DrawSizeGrip(e.Graphics, this.BackColor, rc);
		}

		protected override void WndProc(ref Message m)
		{
			if (m.Msg == WM_NCHITTEST)
			{
				Point pos = new Point(m.LParam.ToInt32() & 0xffff, m.LParam.ToInt32() >> 16);
				pos = this.PointToClient(pos);
				int sectionHorizontal = 0;
				int sectionVertical = 0;
				Int16[,] results = {{HTTOPLEFT, HTTOP, HTTOPRIGHT}, {HTLEFT, HTCAPTION, HTRIGHT}, {HTBOTTOMLEFT, HTBOTTOM, HTBOTTOMRIGHT}};
				if (pos.X < gripSize)
				{
					sectionHorizontal = - 1;
				}
				else if (pos.X >= this.ClientSize.Width - gripSize)
				{
					sectionHorizontal = 1;
				}
				if (pos.Y < gripSize)
				{
					sectionVertical = -1;
				}
				else if (pos.Y >= this.ClientSize.Height - gripSize)
				{
					sectionVertical = 1;
				}
				m.Result = (IntPtr) results[sectionVertical + 1, sectionHorizontal + 1];
				return;
			}
			base.WndProc(ref m);
		}

		public void MainFormKeyDown(object sender, KeyEventArgs e)
		{
			if (e.KeyCode == Keys.Escape)
			{
				Close();
			}
			else if (e.KeyCode == Keys.Enter)
			{
				System.Console.Out.WriteLine(Left.ToString() + "," + Top.ToString() + "," + Width.ToString() + "," + Height.ToString());
				this.success = true;
				Close();
			}
			else if (e.KeyCode == Keys.Up || e.KeyCode == Keys.Right || e.KeyCode == Keys.Down || e.KeyCode == Keys.Left)
			{
				int delta = e.Control ? 1 : movementSpeed;
				if (e.KeyCode == Keys.Left || e.KeyCode == Keys.Up)
				{
					delta *= -1;
				}
				if (e.Shift)
				{
					if (e.KeyCode == Keys.Right || e.KeyCode == Keys.Left)
					{
						Width += delta;
					}
					else
					{
						Height += delta;
					}
				}
				else
				{
					if (e.KeyCode == Keys.Right || e.KeyCode == Keys.Left)
					{
						Left += delta;
					}
					else
					{
						Top += delta;
					}
				}
			}
		}
	}
}
