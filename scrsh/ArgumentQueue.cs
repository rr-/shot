using System;
using System.Collections.Generic;

namespace ScrSh
{
	/// <summary>
	/// Class that represents queue of arguments passed to program.
	/// </summary>
	public class ArgumentQueue : Queue<string>
	{
		public ArgumentQueue(IEnumerable<string> args) : base(args)
		{
		}

		/// <summary>
		/// Try to make exception more helpful.
		/// </summary>
		/// <returns>Dequeued argument.</returns>
		public new string Dequeue()
		{
			if (Count == 0)
				throw new ArgumentException("Unexpected end of argument list");
			return base.Dequeue();
		}
	}
}