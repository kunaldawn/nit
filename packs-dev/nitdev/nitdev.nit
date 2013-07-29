import nit

try f.destroy()

var function id_char(ch: int): bool
{
	return (ch == $'.' || ch == $'_'
		|| ($'A' <= ch && ch <= $'Z')
		|| ($'a' <= ch && ch <= $'z')
		|| ($'0' <= ch && ch <= $'9') )
}

//dump(script.unit)

debug.active = false

app.runtime.debugServer.remote.shutdown()

// reopen remote debug host on other port - to debug debugger itself
//app.runtime.debugServer.listen(null, 1234)
//app.runtime.debugServer.remote.packetDump = true

dofile("nitdebugwin")	// TODO: change dofile to require
dofile("debugclient")	// TODO: change dofile to require

////////////////////////////////////////////////////////////////////////////////

class DataModel
{
	property peer : wx.DataViewModel get _peer
	
	constructor(columns = [])
	{
		_columns = columns
		
		// HACK: 순환 참조가 생기므로 weak()로 하고,
		// 본체가 이 DataModel의 ref를 유지하고 있어야 함.
		_peer = wx.DataViewModel(this.weak())
	}
	
	// wx.DataViewModel implementation ///////////////////////
	
	function getChildren(id: int) : array<int>
	{
		var item = _items[id]
		
		var expander = item.delete("expander")
		
		if (expander)
		{
			costart by {
				_expanders[id] := getthread().weak()
				sleep()
				expander(this, try item.obj, id)
				_expanders.delete(id)
			}
		}
		
		return item.children
	}
	
	function getColumnCount() : int
	{
		return _columns.len()
	}
	
	function getColumnType(col: int): string
	{
		return _columns[col].type
	}
	
	function getParent(id: int) : int
	{
		return try _items[id].parent : 0
	}
	
	function isContainer(id: int): bool
	{
		return try "children" in _items[id] : false
	}
	
	function hasContainerColumns(id: int) : bool
	{
		var item = _items[id]
		return "obj" in item
	}
	
	function getValue(id: int, col: int) : value
	{
		var column = _columns[col]
		
		var obj = try _items[id].obj
		if (!obj) return try _items[id].title
		
		var getter = try obj[column.name]
		
		if (type(getter) != "function") return getter
			
		return try getter(obj)
	}
	
	/*
	o = 
	{
		bp = bp
		title = @(obj) => obj.bp :> format("%s: %s", doc.pack, doc.name)
		text = @(obj, value=null) => value ? obj.bp.text : obj.bp.text = value
	}
	*/
	
	function setValue(id: int, col: int, value): bool
	{
		var column = _columns[col]
		
		var obj = try _items[id].obj
		if (!obj) return false
		
		var setter = try obj[column.name]
		
		if (type(setter) != "function") return setter
		
		try setter(obj, value) catch(ex) { print("*** error: " + ex); return false }
		return true
	}
	
	////////////////////////////////////////////////

	function addColumn(value_type: string, name: string): int
	{
		var column = 
		{
			type = value_type
			name = name
		}
		
		var id = _columns.len()
		
		_columns.push(column)
		
		return id
	}
	
	function clear()
	{
		_items = { [0] = { children=[] } }
		_nextItemID = 1
		
		foreach (expander in _expanders)
			try expander.kill()
		_expanders = {}
		_peer.cleared()
	}
	
	function addItem(obj: object, parent_id: int=0): int
	{
		var parent = _items[parent_id]
		
		if (!"children" in parent)
			parent.children := []
			
		var id = _nextItemID++
		var item = { parent = parent_id, obj = obj }
		
		_items[id] := item
		parent.children.push(id)
		
		_peer.itemAdded(parent_id, id)
		
		return id
	}
	
	function addExpanderItem(obj: object, expander: closure, parent_id: int=0): int
	{
		var parent = _items[parent_id]
		
		if (!"children" in parent)
			parent.children := []
			
		var id = _nextItemID++
		var item = { parent = parent_id, obj = obj, expander = expander, children = []  }
		
		_items[id] := item
		parent.children.push(id)
		
		_peer.itemAdded(parent_id, id)
		
		return id
	}
	
	function hasItem(item_id: int) : bool
	{
		return item_id in _items
	}
	
	function getItem(item_id: int) : int
	{
		return try _items[item_id].obj
	}
	
	function setItem(item_id: int, obj: object)
	{
		var item = _items[item_id]
		
		if (item.obj != obj)
		{
			item.obj = obj
			_peer.itemChanged(item_id)
		}
	}
	
	function addContainer(title: string, parent_id: int=0): int
	{
		var parent = _items[parent_id]
		
		if (!"children" in parent)
			parent.children := []
			
		var id = _nextItemID++
		var item = { parent = parent_id, title = title, children = [] }
		
		_items[id] := item
		parent.children.push(id)

		_peer.itemAdded(parent_id, id)
		
		return id
	}
	
	function deleteItem(item_id: int)
	{
		try _expanders.delete(item_id).kill()

		var item = _items[item_id]
		var parent_id = item.parent
		_peer.itemDeleted(parent_id, item_id)
		
		_items.delete(item_id)
		
		var function subDelete(item)
		{
			var children = item.delete("children")
			if (children)
			{
				foreach (child_id in children)
				{
					try _expanders.delete(child_id).kill()
					var child = _items.delete(child_id)
					subDelete(child)
				}
			}
		}
	
		subDelete(item)
		
		var parent = try _items[parent_id]
		
		if (parent)
		{
			var children = parent.children
			for (var i=0; i<children.len(); ++i)
			{
				if (children[i] == item_id)
				{
					children.remove(i)
					break
				}
			}
		}
		
		if (_items.len() == 1)
			_nextItemID = 1
	}
	
	function toArray()
	{
		var ret = []
		foreach (item in _items)
		{
			try ret.push(item.obj)
		}
		return ret
	}
	
	function _nexti(idx)
	{
		foreach (item in _items)
		{
			// TODO: 'try yield item.obj'도 되는지 검사
			if ("obj" in item)
				yield item.obj
		}
	}
	
	var _columns : array
	var _items = { [0] = { children=[] } }
	var _nextItemID = 1
	var _expanders = { }
	var _peer
}

////////////////////////////////////////////////////////////////////////////////

class Document
{
	property id			 		get _id			// int or given key
	property title	: string 	get _title		set { _title = value; updateTitles() }
	property file 	: string	get _file
	property pack 	: string	get _pack
	property url	: string	get _url
	property crc 	: int		get _crc
	property addr 	: string	get _addr
	
	property readOnly : bool	get _readonly 	set _setReadOnly(value)
	
	property numPanes			get _panes.len()
	property firstPane			get try _panes[0]
	
	property firstEditor		get _getFirstEditor()
	property focusedEditor		get _getFocusedEditor()
	property debugEditor		get _getDebugEditor()
	
	constructor(frame: SolEditFrame, id, params: table)
	{
		_file 	= try params.file
		_pack 	= try params.pack
		_url 	= try params.url
		_crc 	= try params.crc : 0
		_addr 	= try params.addr
	
		_id		= id
		_frame	= frame.weak()
		
		_title	= try params.title
		
		updateTitles()
	}
	
	function addPane(pane: DocumentPane) : DocumentPane
	{
		pane.document = this
		_panes.push(pane)
		
		updateTitles()
		
		return pane
	}
	
	function closePane(pane: DocumentPane, updateNow = true)
	{
		for (var i=0; i < _panes.len(); ++i)
		{
			if (_panes[i] == pane)
			{
				_panes.remove(i)
				break
			}
		}

		var editor = try pane.editor
		
		if (editor == _firstEditor)
			_firstEditor = null
		if (editor == _lastFocused)
			_lastFocused = null
		if (editor == _lastDebug)
			_lastDebug = null
		
		updateTitles()
	}
	
	function updateTitles()
	{
		if (_title == null)
		{
			if (_pack && _file)
				_title = format("%s: %s", _pack, _file)
			else if (_url)
				_title = _url
			else
				_title = format("untitled_#%d", id)
		}
		
		var numPanes = _panes.len()
		
		var no = 1
		
		foreach (pane in _panes)
		{
			var parent = pane.parent
			
			if (parent is wx.BookCtrlBase)
			{
				var title = numPanes == 1 ? _title : format("%s:%d", _title, no++)
				var idx = parent.getPageIndex(pane)
				parent.setPageText(idx, title)
			}
		}
	}
	
	function _getFirstEditor()
	{
		if (_firstEditor) return _firstEditor

		foreach (pane in _panes)
		{
			_firstEditor = try pane.editor
			if (_firstEditor) break
		}
		
		return _firstEditor
	}
	
	function _getFocusedEditor()
	{
		if (!_lastFocused) _lastFocused = firstEditor
		return _lastFocused
	}
	
	function _getDebugEditor()
	{
		if (!_lastDebug) _lastDebug = focusedEditor
		return _lastDebug
	}

	function _setReadOnly(value: bool)
	{
		_readonly = value
		
		foreach (pane in _panes)
		{
			try pane.editor.readOnly = value
		}
	}
	
	function addBreakpoint(bp: Breakpoint)
	{
		_breakpoints[bp.line] := bp.weak()
	}
	
	function getBreakpoint(line: int) : Breakpoint
	{
		return try _breakpoints[line]
	}
	
	function removeBreakpoint(line: int) : Breakpoint
	{
		var bp = _breakpoints.delete(line)
		return bp
	}
	
	var _id

	var _title	: string
	var _file 	: string
	var _pack 	: string
	var _url 	: string
	var _crc 	: int
	var _addr 	: string
	
	var _readonly = false
	
	var _panes = []
	var _frame	: weakref<NitEditFrame>
	var _firstEditor
	var _lastFocused
	var _lastDebug
	
	var _breakpoints = { }	// key: line
}

////////////////////////////////////////////////////////////////////////////////

class Breakpoint
{
	property id			: int		get _id
	property title		: string	get _getTitle()
	property line		: int		get _line

	property document	: Document	get _doc	set _doc = value.weak()
	
	constructor(id, doc, line)
	{
		_id		= id
		_doc	= doc.weak()
		_line	= line
	}
	
	function _getTitle() : string
	{
		if (_doc.pack && _doc.file)
			return format("%s: %s", _doc.pack, _doc.file)
		else
			return _doc.url
	}

	var _id
	var _doc : weak<Document>
	var _line
}
	
////////////////////////////////////////////////////////////////////////////////

class NitEditFrame : wx.ScriptFrame
{
	static ID =
	{
		RESTART			= wx.newId()
		SELF_DEBUG		= wx.newId()
		
		NEW				= wx.ID.NEW
		OPEN			= wx.ID.OPEN
		CLOSE			= wx.newId()
		SAVE			= wx.ID.SAVE
		SAVEAS			= wx.ID.SAVEAS
		SAVEALL			= wx.newId()
		EXIT			= wx.ID.EXIT
		
		AUTOCOMPLETE	= wx.newId()
	
		FIND_TEXT		= wx.ID.FIND
		FIND_NEXT		= wx.newId()
		FIND_PREV		= wx.newId()
		GOTO_LINE		= wx.newId()
		
		CUT				= wx.ID.CUT
		COPY			= wx.ID.COPY
		PASTE			= wx.ID.PASTE
		SELECTALL		= wx.ID.SELECTALL
		UNDO			= wx.ID.UNDO
		REDO			= wx.ID.REDO
		
		SHOW_CONSOLE	= wx.newId()
		
		ATTACH			= wx.newId()
		DETACH			= wx.newId()
		
		STEP_INTO		= wx.newId()
		STEP_OVER		= wx.newId()
		STEP_OUT		= wx.newId()
		CONTINUE		= wx.newId()
		BREAK			= wx.newId()

		TOGGLE_BP		= wx.newId()
		BREAKPOINTS		= wx.newId()
	}
	
	constructor()
	{
		base.constructor(null, wx.ID.ANY, "nit-dev")
		
		var frame = this
		
		bind(EVT.SHOW, frame, onShow)
		bind(EVT.CLOSE, frame, onClose)

		_editorFont = wx.Font :> this(10, 
			FAMILY.TELETYPE, STYLE.NORMAL, WEIGHT.NORMAL, false, "Consolas")
		
		createStatusBar(4) with
		{
			var statusTxtWidth = getTextExtent("OVRW").width
			setStatusWidths([-1, statusTxtWidth, statusTxtWidth, statusTxtWidth*5])
			statusText = "Welcome to nit-dev"
		}
		
		var ID = frame.ID
		
		frame.menuBar = wx.MenuBar()
		
		// File menu /////

		var fileMenu = wx.Menu( [
			[ ID.RESTART,	"&Restart\tCtrl+F7",	"Restart Debugger" ],
			[ ID.SELF_DEBUG, "Self &Debug",			"Enable Self Debug" ],
			null,
			[ ID.NEW, 		"&New\tCtrl+N",			"Create an empty document" ],
			[ ID.OPEN, 		"&Open...\tCtrl+O",		"Open an existing document" ],
			[ ID.CLOSE,		"&Close\tCtrl+W",		"Close the current editor window" ],
			null,
			[ ID.SAVE,		"&Save\tCtrl+S",		"Save the current document" ],
			[ ID.SAVEAS,	"Save &As...\tAlt+S",	"Save the current document to a file with a new name" ],
			[ ID.SAVEALL,	"Save A&ll\tCtrl+Shift+S", "Save all open documents" ],
			null,
			[ ID.EXIT,		"E&xit\tAlt+X",			"Exit Program" ],
		] )
		
		menuBar.append(fileMenu, "&File")
		
		var fileSource = script.unit.source
		bind(EVT.MENU, ID.RESTART, getroottable()) by (evt) => dofile(fileSource.name, fileSource.locator)
		
		bind(EVT.MENU, ID.SELF_DEBUG, frame) by (evt)
		{
			app.runtime.debugServer.listen(null, 1234)
			debug.active = true
		}
		
		bind(EVT.MENU, ID.OPEN, frame, onFileOpen)
		bind(EVT.MENU, ID.EXIT, frame) by (evt) => close()
	
		// TODO: windows 8 mouse cursor disappearing caused by setting evt.enabled = false on EVT.UPDATE_UI
		if (false)
			bind(EVT.UPDATE_UI, ID.EXIT, frame) by (evt) { evt.enabled = false }
		
		// Edit Menu //////
		
		var editMenu = wx.Menu( [
			[ ID.AUTOCOMPLETE, "&Auto Complete\tCtrl+Space",	"Popup an auto-complete window" ],
			null,
			[ ID.FIND_TEXT, "&Find...\tCtrl+F", "Find text"],
			[ ID.FIND_NEXT, "&Find Next\tF3", "Find next position of last search"],
			[ ID.FIND_PREV, "&Find Previous\tShift+F3", "Find previous position of last search"],
			[ ID.GOTO_LINE, "&Go To...\tCtrl+G", "Go to specified line"],
		] )
		
		menuBar.append(editMenu, "&Edit")
		
		bind(EVT.MENU, ID.AUTOCOMPLETE, frame, onEditAutoComplete)
		
		bind(EVT.MENU, ID.FIND_TEXT, frame, onEditFindText)
		bind(EVT.MENU, ID.FIND_NEXT, frame, onEditFindNext)
		bind(EVT.MENU, ID.FIND_PREV, frame, onEditFindPrev)
		bind(EVT.MENU, ID.GOTO_LINE, frame, onEditGotoLine)
	
		// Debug Menu ////////
		
		var debugMenu = wx.Menu( [
			[ ID.SHOW_CONSOLE,	"View &Log Window\tAlt+2",	"View or Hide the log window", wx.ITEM.CHECK ],
			null,
			[ ID.ATTACH,		"&Attach To...",			"Attach to remote" ],
			[ ID.DETACH,		"&Detach Debugger",			"Detach debugger from current remote" ],
			null,
			[ ID.STEP_INTO,		"Step &Into\tF11",			"Step Into" ],
			[ ID.STEP_OVER,		"Step &Over\tF10",			"Step Over" ],
			[ ID.STEP_OUT,		"Step &Out\tShift+F11",		"Step Out" ],
			[ ID.CONTINUE,		"Continue\tF5",				"Continue" ],
			[ ID.BREAK,			"Break",					"Break" ],
			null,
			[ ID.TOGGLE_BP,		"Toggle &Breakpoint\tF9",	"Toggle Breakpoint" ],
			[ ID.BREAKPOINTS,	"Breakpoints\tAlt+F9",		"Show breakpoints window", wx.ITEM.CHECK ],
		])
		
		menuBar with
		{
			append(debugMenu, "&Debug")
			
			enable(ID.STEP_INTO, false)
			enable(ID.STEP_OVER, false)
			enable(ID.STEP_OUT, false)
			enable(ID.CONTINUE, false)
			enable(ID.BREAK, false)
		}
		
		bind(EVT.MENU, ID.SHOW_CONSOLE, frame, onShowConsole)
		
		bind(EVT.MENU, ID.ATTACH, frame, onDebugAttach)
		
		bind(EVT.MENU, ID.STEP_INTO, frame, onDebugStepInto)
		bind(EVT.MENU, ID.STEP_OVER, frame, onDebugStepOver)
		bind(EVT.MENU, ID.STEP_OUT, frame, onDebugStepOut)
		bind(EVT.MENU, ID.CONTINUE, frame, onDebugContinue)
		bind(EVT.MENU, ID.BREAK, frame, onDebugBreak)

		bind(EVT.MENU, ID.TOGGLE_BP, frame) by (evt) => toggleBreakPoint(currDocument, currEditor :> lineFromPosition(currentPos) + 1)
		
		/////////////
		
		_splitter = wx.SplitterWindow(frame) with
		{
			setWindowStyle(STYLE.LIVE_UPDATE)
		}
		
		_docPanes = wx.AuiNotebook(_splitter) with
		{
			windowStyle = windowStyle & ~ STYLE :> (MIDDLE_CLICK_CLOSE | WINDOWLIST_BUTTON)
			auiManager :> flags = flags | STYLE.LIVE_RESIZE
			bind(EVT.PAGE_CLOSE, frame, frame.onDocPaneClose)
		}
		
		_consolePages = wx.AuiNotebook(_splitter) with
		{
			windowStyle = STYLE :> (TAB_SPLIT | TAB_MOVE | SCROLL_BUTTONS | WINDOWLIST_BUTTON)
			auiManager :> flags = flags | STYLE.LIVE_RESIZE
		}
		
		_localConsole = wx.NitConsole(_consolePages) with
		{
			prompt = "local> "
			attachLogger(LogManager.LOG_LEVEL.VERBOSE)
			bind(EVT.COMMAND, frame) by (evt) => script.command(evt.command)
		}
		_consolePages.addPage(_localConsole, "Local Console", true)
		
		_remoteConsole = wx.NitConsole(_consolePages) with
		{
			bind(EVT.COMMAND, frame, frame.onRemoteConsoleCommand)
		}
		_consolePages.addPage(_remoteConsole, "Remote Console")

		_bpmodel = DataModel()
		_bpwindow = createBreakpointWin(_consolePages, _bpmodel)
		_consolePages.addPage(_bpwindow, "Breakpoints")
		
		_callstackModel = DataModel()
		_callstackWin = createCallstackWin(_consolePages, _callstackModel)
		_consolePages.addPage(_callstackWin, "Callstack")
		
		_localsModel = DataModel()
		_localsWin = createLocalsView(_consolePages, _localsModel)
		_consolePages.addPage(_localsWin, "Locals")
		
		_globalsModel = DataModel()
		_globalsWin = createGlobalsView(_consolePages, _globalsModel)
		_consolePages.addPage(_globalsWin, "Globals")
		
		_packsModel = DataModel()
		_packsWin = createPacksView(_consolePages, _packsModel)
		_consolePages.addPage(_packsWin, "packs")
		
		_consolePages.visible = false
		
		_splitter.initialize(_docPanes)
		_splitter.sashGravity = 1.0

		//////////////
		
		// import class name from runtime
		var words = []
		var nss = {}
		foreach (cls in script.getClasses())
		{
			if (!cls) continue
			
			var clsname = cls._classname
			var ns = cls._namespace
			
			if (ns != "" && ns != "unnamed")
			{
				foreach (ns in ns.split('.'))
				{
					if (!(ns in nss))
					{
						words.push(ns)
						nss[ns] := true
					}
				}
			}

			words.push(clsname)
		}
		
		_word2List = words.sort().reduce by (prev, cur) => prev + " " + cur
	}
	
	function createBreakpointWin(parent: wx.Widget, model: DataModel)
	{
		var view = wx.DataViewCtrl(parent)
		
		view with
		{
			appendTextColumn("File",		model.addColumn("string", "title")) with { width = 200 }
			appendTextColumn("Line",		model.addColumn("int", "line")) with { width = 60 }

			associateModel(model.peer)
		}
		
		return view
	}
	
	function createCallstackWin(parent: wx.Widget, model: DataModel)
	{
		var view = wx.DataViewCtrl(parent)
		
		view with
		{
			appendTextColumn("",			model.addColumn("wxIcon", "status")) with { width = 20 }
			appendTextColumn("Name",		model.addColumn("string", "name")) with { width = 120; view.expanderColumn = this }
			appendTextColumn("Location",	model.addColumn("string", "location")) with { width = 180 }

			associateModel(model.peer)
		}
		
		view.bind(view.EVT.ITEM_ACTIVATED, this, onCallstackViewItemActivated)

		return view
	}
	
	function onCallstackViewItemActivated(evt: wx.DataViewEvent)
	{
		var item_id = evt.item
		var obj = _callstackModel.getItem(item_id)
		
		var si = try obj.stackinfo
		
		if (si && _currAttachedAddr)
		{
			var addr 	= _currAttachedAddr
			var pack 	= try si.pack.name
			var file 	= try si.file.name
			var url 	= try si.url.name
			var line 	= si.line
			
			costart by { showEditor(addr, pack, file, url, line) }
			costart by { updateInspector(_localsModel, si.locals) }
		}
	}
	
	function onPacksViewItemActivated(evt: wx.DataViewEvent)
	{
		var item_id = evt.item
		var obj = _packsModel.getItem(item_id)
		
		var addr = _currAttachedAddr
		var pack = try obj.pack.name
		var file = try obj.file.name
		var url = try obj.file.url
		var line = 0
		
		if (addr && pack && file)
		{
			costart by showEditor(addr, pack, file, url, line)
		}
	}
	
	function createLocalsView(parent: wx.Widget, model: DataModel)
	{
		var view = wx.DataViewCtrl(parent)
		
		view with
		{
			appendTextColumn("Kind",		model.addColumn("string", "kind")) with { width = 20 }
			appendTextColumn("Name",		model.addColumn("string", "name")) with { width = 120; view.expanderColumn = this }
			appendTextColumn("Value",		model.addColumn("string", "value")) with { width = 200 }
			appendTextColumn("Type",		model.addColumn("string", "type")) with { width = 80 }
			
			associateModel(model.peer)
		}
		
		return view
	}

	function createGlobalsView(parent: wx.Widget, model: DataModel)
	{
		// HACK: same with localsView
		
		return createLocalsView(parent, model)
	}
	
	function createPacksView(parent: wx.Widget, model: DataModel)
	{
		var view = wx.DataViewCtrl(parent)
		
		view with
		{
			appendTextColumn("Type",		model.addColumn("string", "type")) with { width = 20 }
			appendTextColumn("Name",		model.addColumn("string", "name")) with { width = 120; view.expanderColumn = this }
			
			associateModel(model.peer)
		}

		view.bind(view.EVT.ITEM_ACTIVATED, this, onPacksViewItemActivated)
		
		return view
	}
	
	property currDocPane get
	{
		return _docPanes :> pageCount > 0 ? getPage(selection) : null
	}
	
	property currDocument get 
	{
		var pane = currDocPane
		return try pane.document
	}
	
	property currEditor get
	{
		return try currDocument.focusedEditor
	}
	
	function onShow(evt)
	{
	}
	
	function onClose(evt: wx.CommandEvent)
	{
		if (!saveOnExit(true)) return
		app.stop()
	}
	
	function onFileOpen(evt: wx.CommandEvent)
	{
		var fd = wx.FileDialog(
			this,
			"Open File", 
			null, // default dir
			null, // default file
			"Nit files (*.nit)|*.nit|Text files (*.txt)|*.txt|All files (*)|*") 
			with { windowStyle = STYLE :> (OPEN | FILE_MUST_EXIST) }
		
		if (fd.showModal() == wx.ID.OK)
		{
			try 
				loadFile(fd.path)
			catch (ex)
			{
				wx.messageBox(format("Unable to load file '%s':\n%s", fd.path, ex), "NitEdit Error",
					wx :> (MB.OK | MB.CENTER | ICON.ERROR), this)
			}
		}
		
		fd.destroy()
	}
	
	function loadFile(filePath: string): Document
	{
		var file_text = FileUtil.readFile(filePath)
		
		var fl = FileLocator("")
		var fs = fl.locate(filePath)
		var url = fs.url
		var buf = fs.open().buffer()
		var crc = buf.calcCrc32()
		var text = buf.toString()
		
		var doc = findDocument by (doc) => doc.url == url && doc.CRC == crc
		
		if (doc)
		{
			var editor = doc.focusedEditor
			selectPane(editor.parent)
		}
		else
		{
			doc = newDocument({ url = url, crc = crc })
			var editor = doc.firstEditor
			editor.appendText(text)
			selectPane(editor.parent)
		}
		
		return doc
	}
	
	function selectPane(pane: DocumentPane)
	{
		var pageIndex = _docPanes.getPageIndex(pane)
		if (pageIndex != -1)
			_docPanes.selection = pageIndex
		pane.setFocus()
	}
	
	function onEditAutoComplete(evt: wx.CommandEvent)
	{
		var editor = currEditor
		if (editor == null) return
		
		var pos = editor.currentPos

		while (pos > 0)
		{
			pos = editor.before(pos)
			if (id_char(editor.getCharAt(pos)))
				continue
			
			break
		}
		
		var word = editor.getTextRange(pos, editor.currentPos).strip()
		
		printf("autocomp: '%s'", word)
		
		var tokens = word.split('.')
		
		if (tokens.len())
		{
			var endToken = word.wildcard("*.") ? "" : tokens.pop()

			// populate user list
			var candidates = [ ]
			var prefix = ""
			var ns = getroottable()
			var tidx = 0
			
			while (tidx < tokens.len())
			{
				var token = tokens[tidx]
				if (!token in ns) break
				
				var t = ns[token]

				if (typeof(t) == "instance")
					t = t.getclass()
					
				if (typeof(t) == "table" || typeof(t) == "class")
				{
					prefix += token + "."
					ns = t
					++tidx
				}
				else break
			}
			
			if (endToken != "")
			{
				endToken = endToken.tolower()
				var hidden = endToken[0] == char('_')
				foreach (k, v in ns)
				{
					if (typeof(k) != 'string') continue
					if (k.len() == 0) continue
					if (!hidden && k[0] == char('_')) continue
					if (k.tolower().ascii_find(endToken) == 0)  // TODO: string.startsWith() and string.endsWith() will be useful here
						candidates.push(prefix + k)
				}
			}
			else
			{
				foreach (k, v in ns)
				{
					if (typeof(k) != 'string') continue
					if (k.len() == 0 || k[0] == char('_')) continue
					candidates.push(prefix + k)
				}
			}
			
			var userList = candidates.sort().reduce by (prev, cur) => prev + " " + cur
			if (userList)
				editor.userListShow(1, userList)
		}
	}
	
	function onEditGotoLine(evt: wx.CommandEvent)
	{
		var editor = currEditor
		if (editor == null) return
		
		var line = editor.currentLine + 1
		
		line = wx.getTextFromUser(
			format("Line number (1 - %d):", editor.lineCount), 
			"Go To Line", 
			line.tostring(), this)
		
		if (line == "")
			return
		
		if (try (line = line.tointeger()) : null == null)
			return
			
		editor.ensureVisibleEnforcePolicy(line - 1)
		editor.gotoLine(line - 1)
	}
	
	var _lastFindText = null
	var _lastFindOpt = 0
	
	function onEditFindText(evt: wx.CommandEvent)
	{
		var editor = currEditor
		if (editor == null) return
		
		var text = _lastFindText
		
		if (editor.selectedText.len())
			text = editor.selectedText
		
		text = wx.getTextFromUser(
			"Enter text to find:",
			"Find Text", 
			text, this)
			
		if (text == "")
			return
			
		_lastFindText = text
		
		onEditFindNext(evt)
	}
	
	function onEditFindNext(evt: wx.CommandEvent)
	{
		var editor = currEditor
		if (editor == null) return
		
		var text = _lastFindText
		var opt = _lastFindOpt
		
		if (text == null) return
		
		var start = editor.currentPos
		
		var pos = editor.findText(start, editor.lastPosition, text, opt)
		
		if (pos < 0)
			pos = editor.findText(0, start, text)

		showFoundTextPos(editor, pos, text)
	}
	
	function onEditFindPrev(evt: wx.CommandEvent)
	{
		var editor = currEditor
		if (editor == null) return
		
		var text = _lastFindText
		var opt = _lastFindOpt
		
		if (text == null) return

		var start = editor.currentPos - 1
		
		var pos = editor.findText(start, 0, text, opt)
		
		if (pos < 0)
			pos = editor.findText(editor.lastPosition, start, text, opt)
		
		showFoundTextPos(editor, pos, text)
	}
	
	function showFoundTextPos(editor, pos, text)
	{
		if (pos < 0)
		{
			wx.messageBox("Can't find text: " + text, "Find Text", wx.ICON.WARNING, this)
			return
		}
		
		var line = editor.lineFromPosition(pos)
		
		editor.ensureVisibleEnforcePolicy(line)
		editor.gotoLine(line)
		editor.gotoPos(pos)
		editor.setSelection(pos, pos + text.len())
	}
	
	function onEditorUserListSelection(evt: wx.StyledTextEvent)
	{
		var editor = evt.eventObject
		if (editor == null) return
		
		var pos = editor.currentPos

		while (pos > 0)
		{
			pos = editor.before(pos)
			if (id_char(editor.getCharAt(pos)))
				continue
			
			break
		}
		
		if (!id_char(editor.getCharAt(pos)))
			pos = editor.after(pos)
		
		editor.setSelection(pos, editor.currentPos)
		editor.replaceSelection(evt.text)
	}
	
	function onShowConsole(evt: wx.CommandEvent)
	{
		if (menuBar.isChecked(ID.SHOW_CONSOLE))
		{
			_splitter.splitHorizontally(_docPanes, _consolePages, clientSize.height * 3 / 4)
			_consolePages.currentPage.setFocus()
		}
		else
		{
			_splitter.unsplit(_consolePages)
		}
	}
	
	var _currAttachedAddr: string = null
	var _lastAttachedAddr: string = "127.0.0.1"
	var _debugClient: DebugClient
	var _remote = Remote()
	var _remoteTask = costart by { var remote = _remote.weak(); try while (true) { sleep(); remote.ref().update() }  }
	
	function onDebugAttach(evt: wx.CommandEvent)
	{
		if (_debugClient)
		{
			// TODO: DETACH
			_debugClient = null
		}
		
		var addr = wx.getTextFromUser("Enter IP address[:port]", "Attach To...", _lastAttachedAddr)
		
		if (_remote.connect(addr))
		{
			costart by
			{
				var debugClient = DebugClient(this, _remote.hostPeer)

				try 
				{
					debugClient.requestAttach()
					_debugClient = debugClient
				
					_currAttachedAddr = addr
					_lastAttachedAddr = addr
				}
				catch (ex)
				{
					print("*** " + ex)
					_debugClient = null
					_currAttachedAddr = null
				}
			}
		}
	}

	function onRemoteConsoleCommand(evt: wx.GoalConsoleEvent)
	{
		if (_debugClient)
			_debugClient.notifyCommand(evt.command)
		else
			_remoteConsole.print("[REMOTE] not connected\n", wx.COLOR.RED)
	}
	
	function onDebugBreak(evt: wx.CommandEvent)
	{
		costart by _debugClient ? _debugClient :> request(CMD.RQ_BREAK) : print("*** not connected")
	}
	
	function onDebugStepInto(evt: wx.CommandEvent)
	{
		costart by _debugClient ? _debugClient :> request(CMD.RQ_STEP_INTO) : print("*** not connected")
	}
	
	function onDebugStepOver(evt: wx.CommandEvent)
	{
		costart by _debugClient ? _debugClient :> request(CMD.RQ_STEP_OVER) : print("*** not connected")
	}
	
	function onDebugStepOut(evt: wx.CommandEvent)
	{
		costart by _debugClient ? _debugClient :> request(CMD.RQ_STEP_OUT) : print("*** not connected")
	}
	
	function onDebugContinue(evt: wx.CommandEvent)
	{
		costart by _debugClient ? _debugClient :> request(CMD.RQ_GO) : print("*** not connected")
	}
	
	function onServerActive(evt: RemoteNotifyEvent)
	{
		menuBar.enable(ID.BREAK, true)
		
		if (_debugClient)
		{
			_debugClient.clearBreakpoints()
			foreach (bp in _bpmodel)
			{
				_debugClient.addBreakpoint(bp)
			}
		}
	}
	
	function onServerLogEntry(evt: RemoteNotifyEvent)
	{
		_remoteConsole.printRemoteLog(evt)
	}
	
	var _lastDebugPane
	var _lastShowEditor
	function resetDebugPaneState()
	{
		if (_lastDebugPane)
		{
			_lastDebugPane.infoBar.dismiss()
			_lastDebugPane.editor.markerDeleteAll(MARKER.CURRENT_LINE)
			_lastDebugPane = null
		}
		
		if (_lastShowEditor)
		{
			_lastShowEditor.markerDeleteAll(MARKER.CURRENT_LINE)
			_lastShowEditor = null
		}
	}
	
	function onServerBreak(evt: RemoteNotifyEvent)
	{
		var params = evt.param()
		
		::p := params // TODO: debug purpose

//		printf("++ break: %s", params.toJson(false))
	
		menuBar.enable(ID.STEP_INTO, true)
		menuBar.enable(ID.STEP_OVER, true)
		menuBar.enable(ID.STEP_OUT, true)
		menuBar.enable(ID.CONTINUE, true)
		
		menuBar.enable(ID.BREAK, false)
		
		costart by
		{
			var addr = _currAttachedAddr
			var pack = try params.pack.name
			var file = try params.file.name
			var url  = try params.url.name : try params.src
			var func = try params.func
			var line = try params.line
			
			var pane = showEditor(addr, pack, file, url, line)
			if (pane == null) return;
			
			var editor = pane.editor
			
			resetDebugPaneState()
			_lastDebugPane = pane
			
			if (params.error)
				pane.infoBar.showMessage(format("%s: %s() at line %d: %s", params.reason, func, line, params.error), wx.ICON.ERROR)
			else
				pane.infoBar.showMessage(format("%s: %s() at line %d", params.reason, func, line), wx.ICON.INFORMATION)

			// HACK: infobar가 자리를 차지하므로, infobar 보여주고 EnsureVisible() 다시 호출
			editor.markerAdd(line - 1, MARKER.CURRENT_LINE)
			editor.ensureVisibleEnforcePolicy(line - 1)
		}

		updateDebugWindows(params)
	}
	
	function showEditor(addr, pack, file, url, line=null) : DocumentPane
	{
		var id = format("%s> %s: %s", addr, pack, file)
		print(id)
		
		var doc = getDocument(id)
		var editor
		
		if (doc == null)
			doc = findDocument by (doc) {
				if (pack)
				{
					var url = try doc.url.tolower().replace("\\", "/")
					var packPos = url ? url.find(pack.tolower()) : null
					var filePos = url ? url.find(file.tolower()) : null
					
					return packPos && filePos && packPos < filePos
				}
				else return url == doc.url
			}
		
		var w = MemoryBuffer.Writer()
		var fi
		
		try
		{
			if (doc)
				fi = _debugClient.requestFile(w, pack, file, doc.crc)
			else
				fi = _debugClient.requestFile(w, pack, file, 0)
			
//			dump(fi)
		}
		catch (ex)
		{
			if (!_lastDebugPane)
				_lastDebugPane = currDocPane

			if (_lastDebugPane)
				_lastDebugPane.infoBar.showMessage(
					format("can't download source '%s: %s' from '%s'", pack, file, addr)
					, wx.ICON.ERROR)
				
			return
		}
		
		if (fi)
		{
			var crc = w.buffer.calcCrc32()
			
			doc = newDocument({
				pack = pack
				file = file
				crc  = crc
				addr = addr
				url  = url
			}, id)
			editor = doc.debugEditor
			editor.appendText(w.buffer.toString())
			doc.readOnly = true
		}
		else
		{
			editor = doc.debugEditor
			doc.title = format("%s: %s", pack, file)
			doc.readOnly = true
		}
		
		if (_lastShowEditor && line != null)
			_lastShowEditor.markerDeleteAll(MARKER.CURRENT_LINE)
		
		_lastShowEditor = editor
		
		if (line != null)
		{
			editor.markerAdd(line - 1, MARKER.CURRENT_LINE)
			editor.ensureVisibleEnforcePolicy(line - 1)
		}
		
		var pane = editor.parent
		
		selectPane(pane)
		
		return pane
	}
	
	function updateDebugWindows(params)
	{
		costart by { updateCallstack(params) }
		costart by { updateInspector(_localsModel, params.threads[0].callstack[0].locals) }
		costart by { updateInspector(_globalsModel, params.globals) }
	}
	
	function updatePacks()
	{
		costart by
		{
			var params = _debugClient :> request(CMD.RQ_PACKS)()
			
			var model = _packsModel
			
			model.clear()
			
			foreach (packname in params.packs.keys().sort())
			{
				var pack = params.packs[packname]
				var packItem =
				{
					name = pack.name
					type = pack.type
					pack = pack
				}
				
				var pack_id = model.addItem(packItem)
				
				foreach (file in pack.files.sort by (a, b) => a.name <=> b.name)
				{
					var fileItem =
					{
						name = file.name
						type = ContentType.fromMimeType(file.mime).name
						pack = pack
						file = file
					}
					
					var file_id = model.addItem(fileItem, pack_id)
				}
			}
		}
	}
	
	function updateCallstack(params)
	{
		var model = _callstackModel
		
		model.clear()
		
		foreach (th in params.threads)
		{
			var current = params.thread == th.thread_id
			var threadItem =
			{
				thread = th
				status = current ? "active" : "suspended"
				name = format("thread: %s", th.name)
				location = ""
			}
			
			var th_id = model.addItem(threadItem)
			
			foreach (si in th.callstack)
			{
				var stackItem =
				{
					stackinfo = si
					status = ""
					name = format("%s.%s()", si.this_name, si.func)
					location = try format("%s: %s #%d", si.pack.name, si.file.name, si.line) : "(unknown)"
				}
				
				var st_id = model.addItem(stackItem, th_id)
			}
			
			if (current)
				_callstackWin.expand(th_id)
		}
	}
	
	function addInspectItem(model, name, o, parent_id = 0)
	{
		var kind = try o.kind : "item"
		var type = o.type
		var value = o.value
		
		if (type == "string")
			value = '"' + value.replace('\n', '\\n') + '"'
			
		if (type == "closure" || type == "n-closure")
			name += "()"
		
		if (try o.truncated)
			value += "..."
		
		var item =
		{
			inspect_id = try o.inspect_id
			kind = kind
			name = name
			value = value
			type = type
		}
		
		if (item.inspect_id)
			model.addExpanderItem(item, onExpandInspector.bind(weak()), parent_id)
		else
			model.addItem(item, parent_id)
	}
	
	function updateInspector(model, items)
	{
		model.clear()
		
		var sortedNames = items.keys().sort()
		foreach (name in sortedNames)
		{
			var o = items[name]
			addInspectItem(model, name, o)
		}
	}
	
	function onExpandInspector(model, item, parent_id)
	{
		costart by
		{
			var members = _debugClient.requestInspect(item.inspect_id)()

			var sortedNames = members.keys().sort()
				
			foreach (name in sortedNames)
			{
				var o = members[name]
				addInspectItem(model, name, o, parent_id)
			}
		}
	}

	function onServerResume(evt: RemoteNotifyEvent)
	{
		menuBar.enable(ID.STEP_INTO, false)
		menuBar.enable(ID.STEP_OVER, false)
		menuBar.enable(ID.STEP_OUT, false)
		menuBar.enable(ID.CONTINUE, false)
		
		menuBar.enable(ID.BREAK, true)

		resetDebugPaneState()
	}
	
	function onServerInactive(evt: RemoteNotifyEvent)
	{
		menuBar.enable(ID.BREAK, false)
	}
	
	function onServerShutdown(evt: RemoteNotifyEvent)
	{
	}
	
	static MARKER =
	{
		BREAKPOINT 		= 1
		CURRENT_LINE 	= 2
	}
	
	function toggleBreakPoint(doc: Document, line: int)
	{
		var editor = doc.firstEditor
		var markers = editor.markerGet(line - 1)
		
		var on = (markers & (1 << MARKER.BREAKPOINT)) == 0
		
		if (on)
		{
			var bp_id = _bpmodel.addItem(null)
		
			var bp = Breakpoint(bp_id, doc, line)
			_bpmodel.setItem(bp_id, bp)
			
			doc.addBreakpoint(bp)
			editor.markerAdd(line - 1, MARKER.BREAKPOINT)

			if (_debugClient) _debugClient.addBreakpoint(bp)
		}
		else
		{
			var bp = doc.removeBreakpoint(line)
			editor.markerDelete(line - 1, MARKER.BREAKPOINT)
			
			if (bp)
			{
				if (_debugClient) _debugClient.removeBreakpoint(bp)
				_bpmodel.deleteItem(bp.id)
			}
		}
	}
	
	function clearBreakpoints(doc: Document)
	{
		var editor = doc.firstEditor
		
		foreach (line, bp in doc._breakpoints)
		{
			if (editor)
				editor.markerDelete(line - 1, MARKER.BREAKPOINT)
			_bpmodel.deleteItem(bp.id)
		}
	}
	
	function saveOnExit(canCancel: bool) : bool
	{
//		wx.messageBox("can't quit!"); return false
		return true
	}
	
	function _createEditorPane(name: string): DocumentPane
	{
		var pane = DocumentPane(_docPanes)
		var editor = pane.editor

		var font = _editorFont
		var frame = this
		
		editor with
		{
			for (var i=0; i <= 32; ++i)
				styleSetFont(i, font)

			lexer = LEX.CPP
				
			useTabs = true
			tabWidth = 4
			indent = 4
			indentationGuides = INDENT_GUIDE.LOOKBOTH
				
			setMarginWidth(0, textWidth(STYLE_TEXT.LINENUMBER, "0000")) // line number margin
			
			setMarginWidth(1, 16) // marker margin
			setMarginType(1, MARGIN.SYMBOL)
			setMarginSensitive(1, true)
			
			styleSetForeColor(STYLE_CPP.COMMENTLINE,	0x008000)
			styleSetForeColor(STYLE_CPP.COMMENT,		0x008000)
			styleSetForeColor(STYLE_CPP.OPERATOR,		0x400040)
			styleSetForeColor(STYLE_CPP.NUMBER,  		0x008080)
			styleSetForeColor(STYLE_CPP.STRING,  		0x800000)
			styleSetForeColor(STYLE_CPP.VERBATIM,		0x800000)
			styleSetForeColor(STYLE_CPP.PREPROCESSOR,	0xC000C0)

			styleSetForeColor(STYLE_CPP.WORD, 	 		0x0000C0)
			styleSetForeColor(STYLE_CPP.WORD2, 	 		0x000080)
			styleSetForeColor(STYLE_CPP.IDENTIFIER,		0x000000)
			
			styleSetForeColor(STYLE_TEXT.INDENTGUIDE, 	0xC0C0C0)
			
			styleSetBackColor(STYLE_TEXT.BRACELIGHT,	0xFFFF80)
			styleSetBackColor(STYLE_TEXT.BRACEBAD,		0xFF8080)
		
			setKeywords(0, 
				@"base break by case catch char class clone constructor continue
				  default destructor div do else false finally for foreach function get
				  if import in is mod null property return resume require set static switch
				  this throw true try typeof var while yield with
				  "@)
			
			markerDefine(frame.MARKER.BREAKPOINT, MARKER.CIRCLE, wx.COLOR.WHITE, wx.COLOR.RED)
			markerDefine(frame.MARKER.CURRENT_LINE, MARKER.ARROW, wx.COLOR.BLACK, wx.COLOR.GREEN)
			
			setMarginWidth(2, 16) // fold margin
			setMarginType(2, MARGIN.SYMBOL)
			setMarginMask(2, MARKNUM.MASK_FOLDERS)
			setMarginSensitive(2, true)
			
			setVisiblePolicy(editor.CARET_POLICY.SLOP, 6)
			
			setFoldFlags(FOLDFLAG.LINEAFTER_CONTRACTED)
			
			setProperty("fold", "1")
			setProperty("fold.compact", "1")
			setProperty("fold.comment", "1")
			
			markerDefine(MARKNUM.FOLDEROPEN, 	MARKER.BOXMINUS, 	wx.COLOR.WHITE, wx.COLOR.LIGHTGREY)
			markerDefine(MARKNUM.FOLDER,		MARKER.BOXPLUS,		wx.COLOR.WHITE, wx.COLOR.LIGHTGREY)
			markerDefine(MARKNUM.FOLDERSUB,		MARKER.VLINE,		wx.COLOR.WHITE, wx.COLOR.LIGHTGREY)
			markerDefine(MARKNUM.FOLDERTAIL,	MARKER.LCORNER,		wx.COLOR.WHITE, wx.COLOR.LIGHTGREY)
			markerDefine(MARKNUM.FOLDEREND,		MARKER.BOXPLUSCONNECTED, wx.COLOR.WHITE, wx.COLOR.LIGHTGREY)
			markerDefine(MARKNUM.FOLDEROPENMID,	MARKER.BOXMINUSCONNECTED, wx.COLOR.WHITE, wx.COLOR.LIGHTGREY)
			markerDefine(MARKNUM.FOLDERMIDTAIL,	MARKER.TCORNER,		wx.COLOR.WHITE, wx.COLOR.LIGHTGREY)
		}

		editor.setKeywords(1, _word2List)
		
		editor.bind(editor.EVT.MARGINCLICK, this) by (evt)
		{
			var line = editor.lineFromPosition(evt.position)
			switch (evt.margin)
			{
			case 1:
				// bp margin
				var pane = editor.parent
				var doc = pane.document
				toggleBreakPoint(doc, line + 1)
				break
				
			case 2:
				// fold margin
				var level = editor.getFoldLevel(line)
				if (level & editor.FOLDLEVEL.HEADERFLAG)
					editor.toggleFold(line)
				break
			}
		}
		
		editor.bind(editor.EVT.USERLISTSELECTION, this, onEditorUserListSelection)
		
		_docPanes.addPage(pane, name, true)
		
		return pane
	}
	
	var _nextDocumentId = 1
	var _documents = { }
	
	function newDocument(params: table, id=null) : Document
	{
		if (id == null) 
			id = _nextDocumentId++
		
		var doc = Document(this, id, params)
		
		_documents[doc.id] := doc
		
		doc.addPane(_createEditorPane(doc.title))
		
		return doc
	}
	
	function newEditorPane(doc: Document) : DocumentPane
	{
		var editor = doc.firstEditor
		var newPane = doc.addPane(_createEditorPane(doc.title))
		newPane.editor.docPointer = editor.docPointer
		
		return newPane
	}
	
	function onDocPaneClose(evt: wx.AuiNotebookEvent)
	{
		var pane = _docPanes :> getPage(selection)
		var doc = try pane.document
		
		if (doc)
		{
			var numPanes = doc.numPanes
			if (numPanes == 1)
			{
				// TODO: save 점검 후 veto
			}
			
			doc.closePane(pane)
			
			if (numPanes == 1)
			{
				closeDocument(doc, false)
			}
		}
	}
	
	function closeDocument(doc: Document, closePanes=true)
	{
		if (closePanes)
		{
			// TODO: 열려 있는 모든 pane 닫기
		}
		
		clearBreakpoints(doc)
		_documents.delete(doc.id)
	}
	
	function getDocument(id)
	{
		return try _documents[id]
	}
	
	function findDocument(pred: closure) : Document
	{
		foreach (doc in _documents)
		{
			if (pred(doc)) return doc
		}
	}
	
	function findAllDocuments(pred: closure) : Document
	{
		var docs = []
		foreach (doc in _documents)
			if (pred(doc)) docs.push(doc)
		return docs
	}

	var _word2List
	
	var _editorFont
	
	var _splitter
	var _docPanes
	var _consolePages
	var _localConsole
	var _remoteConsole
	
	var _bpwindow : wx.DataViewCtrl
	var _bpmodel : DataModel
	
	var _callstackWin : wx.DataViewCtrl
	var _callstackModel : DataModel
	
	var _localsWin : wx.DataViewCtrl
	var _localsModel : DataModel
	
	var _globalsWin : wx.DataViewCtrl 
	var _globalsModel : DataModel
	
	var _packsWin : wx.DataViewCtrl
	var _packsModel : DataModel
}

f := NitEditFrame()
//f.newDocument({ title = "untitled.nit" })
f.size = wx.Size(960, 720)
f.center()
f.show()
