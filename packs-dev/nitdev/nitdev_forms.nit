// NIT code generated with wxFormBuilder for nit (version Apr  3 2013)
// PLEASE DO "NOT" EDIT THIS FILE!


////////////////////////////////////////////////////////////////////////////////

// Class DocumentPane_Form

class DocumentPane_Form : wx.ScriptPanel
{
	constructor(parent=null, id=wx.ID.ANY, pack=script.locator)
	{
		base.constructor(parent, id, wx.DEFAULT.POS, wx.Size(618,515), wx.Window.STYLE.TAB_TRAVERSAL)
		
		var _bSizer1 = wx.BoxSizer(wx.VERTICAL)
		
		_navPanel = wx.Panel(this, wx.ID.ANY, wx.DEFAULT.POS, wx.DEFAULT.SIZE, wx.Window.STYLE.TAB_TRAVERSAL)
		var _bSizer2 = wx.BoxSizer(wx.HORIZONTAL)
		
		_navTitle = wx.StaticText(_navPanel, wx.ID.ANY, "Members:", wx.DEFAULT.POS, wx.DEFAULT.SIZE, 0)
		_navTitle.wrap(-1)
		_bSizer2.add(_navTitle, 0, wx.ALIGN.CENTER_VERTICAL | wx.DIR.RIGHT, 5)
		
		var _comboBox1 = wx.ComboBox(_navPanel, wx.ID.ANY, "Combo!", wx.DEFAULT.POS, wx.DEFAULT.SIZE, [  ], 0)
		_bSizer2.add(_comboBox1, 1, wx.ALIGN.CENTER_VERTICAL | wx.DIR.LEFT, 5)
		
		
		_navPanel.sizer = _bSizer2
		_navPanel.layout()
		_bSizer2.fit(_navPanel)
		
		_bSizer1.add(_navPanel, 0, wx.STRETCH.EXPAND | wx.DIR.ALL, 5)
		
		var _staticline1 = wx.StaticLine(this, wx.ID.ANY, wx.DEFAULT.POS, wx.DEFAULT.SIZE, wx.StaticLine.STYLE.HORIZONTAL)
		_bSizer1.add(_staticline1, 0, wx.STRETCH.EXPAND | wx.DIR.BOTTOM, 5)
		
		_editor = wx.StyledTextCtrl(this, wx.ID.ANY, wx.DEFAULT.POS, wx.DEFAULT.SIZE, 0|wx.Window.STYLE.VSCROLL | wx.Window.STYLE.HSCROLL | wx.BORDER.NONE | wx.Window.STYLE.WANTS_CHARS)
		_bSizer1.add(_editor, 1, wx.STRETCH.EXPAND, 5)
		
		_infobar = wx.InfoBar(this, wx.ID.ANY)
		_infobar.setShowHideEffects(wx.SHOW_EFFECT.NONE, wx.SHOW_EFFECT.NONE)
		_bSizer1.add(_infobar, 0, wx.ALIGN.CENTER_VERTICAL | wx.STRETCH.EXPAND, 5)
		
		
		this.sizer = _bSizer1
		this.layout()
	}
	
	destructor()
	{
	}
	
	
	// Member Declarations
	var _navPanel : wx.Panel
	var _navTitle : wx.StaticText
	var _editor : wx.StyledTextCtrl
	var _infobar : wx.InfoBar
	
}
