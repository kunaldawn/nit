import nit

////////////////////////////////////////////////////////////////////////////////

class DebugClient
{
	static _packetLog = false
	
	static CMD =
	{
		RQ_ATTACH			= 0x0001,
		NT_DETACH			= 0x0002,
		
		NT_COMMAND			= 0x0010,
		
		RQ_PACKS			= 0x0020,
		RQ_FILE				= 0x0021,
		
		NT_CLEAR_BP			= 0x0030,
		NT_ADD_BP			= 0x0031,
		NT_DEL_BP			= 0x0032,
		
		NT_CLEAR_WATCH		= 0x0040,
		NT_ADD_WATCH		= 0x0041,
		NT_DEL_WATCH		= 0x0042,
		
		RQ_BREAK			= 0x0050,
		
		RQ_GO				= 0x0051,
		RQ_STEP_INTO		= 0x0052,
		RQ_STEP_OVER		= 0x0053,
		RQ_STEP_OUT			= 0x0054,
		
		RQ_LOCALS			= 0x0060,
		RQ_INSPECT			= 0x0061,
		RQ_EVALUATE			= 0x0062,
		
		NT_SVR_ACTIVE		= 0x1001,
		NT_SVR_LOG_ENTRY	= 0x1002,
		NT_SVR_BREAK		= 0x1003,
		NT_SVR_RESUME		= 0x1004,
		NT_SVR_INACTIVE		= 0x1005,
		NT_SVR_SHUTDOWN		= 0x1006,
		
		UP_SVR_FILE			= 0x1021,
	}
	
	static RESPONSE =
	{
		OK		= Remote.RESPONSE.OK
		ERROR	= Remote.RESPONSE.ERROR
		
		FILE_SAME		= 1
		FILE_NOT_FOUND	= -101
		FILE_ERROR		= -102
	}
	
	constructor(client, peer: RemotePeer, channelId = 0xdeb6)
	{
		_channelId = channelId
		
		_client = client.weak()
		
		_peer = peer
		var remote = _peer.remote
		
		remote.closeChannel(channelId)

		var ch = remote.openChannel(channelId, "nit.RemoteDebugClient")
		
		ch.bind(remote.EVT.NOTIFY, this, onNotify)
		ch.bind(remote.EVT.RESPONSE, this, onResponse)
		ch.bind(remote.EVT.UPLOAD_START, this, onUploadStart)
		
		_notifyHandlers =
		{
			[CMD.NT_SVR_ACTIVE] 	= _client.onServerActive
			[CMD.NT_SVR_LOG_ENTRY] 	= _client.onServerLogEntry
			[CMD.NT_SVR_BREAK] 		= _client.onServerBreak
			[CMD.NT_SVR_RESUME] 	= _client.onServerResume
			[CMD.NT_SVR_INACTIVE] 	= _client.onServerInactive
			[CMD.NT_SVR_SHUTDOWN] 	= _client.onServerShutdown
		}
	}
	
	destructor()
	{
		try _peer.remote.closeChannel(_channelId)
	}

	var _notifyHandlers = { }
	
	function onNotify(evt: RemoteNotifyEvent)
	{
		if (_packetLog)
			printf(".. [dbgcli] nt cmd 0x%04X", evt.Command)
		
		var handler = try _notifyHandlers[evt.Command]
		if (handler)
			handler.call(_client, evt)
		else
			throw format("can't find notify handler for cmd 0x%04X", evt.command)
	}
	
	function onResponse(evt: RemoteResponseEvent)
	{
		var rqid = evt.requestID
		var req = try _requests[rqid]
		
		if (req) 
		{
			try req.onResponse(evt)
			_requests.delete(rqid)
		}
	}
	
	class RequestEntry
	{
		var command: CMD
		var params
		var requestID: int
		var code: int
		var result
		var _wb: WaitBlock
		
		function wait()
		{
			_wb.wait()
		}
	}
	
	function request(cmd: CMD, params=null, codeHandler=null) : table
	{
		var rq_id = _peer.remote.request(_Peer, _channelId, cmd, params)
		var code = null
		var ret = null
		var wb = nit.WaitBlock()
		
		var request =
		{
			function onResponse(evt: RemoteResponseEvent)
			{
				code = evt.code
				ret = evt.packet.readValue()
				wb.signal(1)
			}
		}
		
		_requests[rq_id] := request

		wb.wWait()

		if (_packetLog)
			printf(".. [dbgcli] rq #%d cmd 0x%04X: code %d, ret: %s", rq_id, cmd, code, ret())
		
		if (codeHandler)
		{
			codeHandler(code, ret)
		}
		
		return ret
	}
	
	function requestAttach()
	{
		return request(CMD.RQ_ATTACH, { client = "SolEdit", version = "1.0", log = true }) by (code, ret)
		{ 
			if (code < 0) throw
				format("can't attach (code: %d): %s", code, ret())
		}
	}
	
	function notifyCommand(cmd: string)
	{
		_peer.remote.notify(_peer, _channelId, CMD.NT_COMMAND, cmd)
	}
	
	function addBreakpoint(bp: Breakpoint)
	{
		var params =
		{
			type	= DataKey.global("sol")
			id		= bp.id
			file 	= bp.document.file
			pack 	= bp.document.pack
			url 	= bp.document.url
			line 	= bp.line
		}
		
		_peer.remote.notify(_peer, _channelId, CMD.NT_ADD_BP, params)
	}
	
	function removeBreakpoint(bp: Breakpoint)
	{
		var params =
		{
			type	= DataKey.global("sol")
			id 		= bp.id
		}
		
		_peer.remote.notify(_peer, _channelId, CMD.NT_DEL_BP, params)
	}
	
	function clearBreakpoints()
	{
		var params =
		{
			type	= DataKey.global("sol")
		}
		
		_peer.remote.notify(_peer, _channelId, CMD.NT_CLEAR_BP, params)
	}
	
	class FileInfo
	{
		var pack		: string
		var file		: string
		var crc			: int
		var contentType	: ContentType
	}
	
	function requestFile(writer: StreamWriter, pack: string, file: string, crc: int): FileInfo
	{
		var params = 
		{
			pack = pack
			file = file
			remote_crc = crc
		}
		
		var req_id = _peer.remote.request(_peer, _channelId, CMD.RQ_FILE, params)
		var wb = nit.WaitBlock()
		
		var code = null
		var ret = null
		var fileInfo = null
		
		var request = 
		{
			function onResponse(evt: RemoteResponseEvent)
			{
				code = evt.code
				ret = evt.packet.readValue()
				wb.signal(1)
			}
			
			function onDownload(evt: RemoteDownloadStartEvent, rec: DataValue)
			{
				evt.download(writer)
				
				try
				{
					fileInfo = DebugClient.FileInfo() with
					{
						pack 			= rec.pack
						file 			= rec.file
						crc 			= rec.local_crc
						contentType		= ::ContentType.fromValue(rec.content_type)
					}
				} catch (ex) print("*** OnDownload: " + ex)
			}
		}
		
		_requests[req_id] := request
		
		wb.wait()
		
		switch (code)
		{
		case 0: return fileInfo
		case 1: return null // same file
		
		case null:
			throw "time-out or canceled"
			
		default:
			throw format("can't get file (code: %d): %s", code, ret())
		}
	}
	
	function onUploadStart(evt: RemoteUploadStartEvent)
	{
		var rec = evt.packet.readValue()
		
		var rq_id = evt.requestID
		var req = try _requests[rq_id]
		
		if (_packetLog)
			printf("++ upload recv: rq_id: %d req: %s, rec: %s", rq_id, req, rec.toJson(false))
		
		if (req)
		{
			try req.onDownload(evt, rec)
		}
	}

	var _client
	var _channelId: int
	var _peer: RemotePeer
	var _requests: table<Request> = { }
}

function dntest()
{
	var t = TimeWatch("dntest")
	
	app.runtime.debugServer.remote.packetDump = false
	w := MemoryBuffer.Writer()
	fi := f._debugClient.requestFile(w, "nit", "baselib.nit", 1)
	dump(fi)
}

