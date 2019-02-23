using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace FilePackager.Undo
{
	interface IUndoEvent
	{
		void Undo();
		void Redo();
	}
}
