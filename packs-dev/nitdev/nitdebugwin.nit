require "nitdev_forms"

////////////////////////////////////////////////////////////////////////////////

class DocumentPane : DocumentPane_Form
{
	property document: Document 		get _document set _document = value.weak()
	property editor: wx.StyledTextCtrl	get _editor
	property infoBar: wx.InfoBar		get _infobar
	
	static EDITOR_FONT = wx.Font :> new(10,
		FAMILY.TELETYPE, STYLE.NORMAL, WEIGHT.NORMAL, false, "Consolas")
	
	constructor(parent: wx.Window, id=wx.ID.ANY)
	{
		base.constructor(parent, id)
		
		bind(EVT.SET_FOCUS, this) by (evt) => _editor.SetFocus()
		_editor.setFocus()
	}
	
	function getPane(editor: wx.StyledTextCtrl)
	{
		return editor.clientObject
	}
	
	var _document : Document
}