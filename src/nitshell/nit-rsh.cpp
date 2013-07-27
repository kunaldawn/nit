/// nit - Noriter Framework
/// A Cross-platform Open Source Integration for Game-oriented Apps
///
/// http://www.github.com/ellongrey/nit
///
/// Copyright (c) 2013 by Jun-hyeok Jang
/// 
/// (see each file to see the different copyright owners)
/// 
/// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal
/// in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
/// copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
/// 
/// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
/// 
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
/// THE SOFTWARE.
///
/// Author: ellongrey

#include "nit/nit.h"

#include "nit/io/Stream.h"
#include "nit/runtime/MemManager.h"
#include "nit/net/Remote.h"
#include "nit/net/RemoteDebugger.h"

#include "ConsoleWindow.h"

#pragma warning (disable: 4996)

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////////////////

class ConsoleWindowLogger : public Logger
{
public:
	ConsoleWindowLogger(ConsoleWindow* console)
	{
		_console = console;
	}

	virtual void doLog(const LogEntry* entry)
	{
		if (_lineStart)
		{
			String str(entry->message, entry->messageLen);
			if (entry->lineEnd)
				str += "\n";
			_console->printString(str.c_str());
		}
		else
		{
			String str(entry->message, entry->messageLen);
			if (entry->lineEnd)
				str += "\n";
			_console->printString(str.c_str());
		}
	}

private:
	ConsoleWindow* _console;
};

////////////////////////////////////////////////////////////////////////////////////////////

class ConsoleApp : public WeakSupported, public ConsoleWindow::IListener
{
public:
	Remote								_remote;
	String								_lastAddr;
	Ref<Logger>							_logger;

	ConsoleWindow*						_console;

	uint16								_channelID;

	enum
	{
		RQ_ATTACH						= DebugServer::RQ_ATTACH,

		NT_COMMAND						= DebugServer::NT_COMMAND,

		NT_SVR_LOG_ENTRY				= DebugServer::NT_SVR_LOG_ENTRY,
		NT_SVR_BREAK					= DebugServer::NT_SVR_BREAK,
		NT_SVR_RESUME					= DebugServer::NT_SVR_RESUME,
	};

	ConsoleApp(ConsoleWindow* cw)
	{ 
		_console = cw;
		cw->setListener(this);

		_channelID = 0xdeb6;

		EventChannel* ch = _remote.getChannel(0);
		ch->bind(EVT::REMOTE_CONNECT, this, &ConsoleApp::onRemoteConnect);
		ch->bind(EVT::REMOTE_DISCONNECT, this, &ConsoleApp::onRemoteDisconnect);

		ch = _remote.openChannel(_channelID, "nit.RemoteDebugClient");

		ch->bind(EVT::REMOTE_NOTIFY, this, &ConsoleApp::onRemoteNotify);
		ch->bind(EVT::REMOTE_RESPONSE, this, &ConsoleApp::onRemoteResponse);
	}

	~ConsoleApp()						{ }

	void onRemoteConnect(const RemoteEvent* evt)
	{
		_console->setPrompt("attaching> ");
	}

	void onRemoteDisconnect(const RemoteEvent* evt)
	{
		_console->setPrompt("idle> ");
	}

	void onRemoteNotify(const RemoteNotifyEvent* evt)
	{
		switch (evt->command)
		{
		case NT_SVR_LOG_ENTRY:			
			onServerLogEntry(evt); 
			break;

		case NT_SVR_BREAK:
			_console->setPrompt("debug> ");
			break;

		case NT_SVR_RESUME:
			_console->setPrompt("> ");
			break;
		}
	}

	void onRemoteResponse(const RemoteResponseEvent* evt)
	{
		switch (evt->command)
		{
		case RQ_ATTACH:
			if (evt->code < 0)
			{
				String msg = evt->param.toString();
				LOG(0, "[nit-rsh] Can't attach debugger (%d) %s\n", evt->code, msg.c_str());
				evt->peer->disconnect();
				break;
			}
			else
			{
				_console->setPrompt("> ");
			}
		}
	}

	void onServerLogEntry(const RemoteNotifyEvent* evt)
	{
		size_t blobSize;
		const void* blob = evt->param.toBlob(&blobSize);

		RemoteLogEntry entry(blob, blobSize);

		COLORREF color;

		switch (entry.logLevel)
		{
		case LOG_LEVEL_VERBOSE:			color = RGB(0x80, 0x80, 0x80); break;
		case LOG_LEVEL_DEBUG:			color = RGB(0x00, 0x00, 0x00); break;
		case LOG_LEVEL_INFO:			color = RGB(0x00, 0x80, 0x00); break;
		case LOG_LEVEL_WARNING:			color = RGB(0xFF, 0x00, 0xFF); break;
		case LOG_LEVEL_ERROR:			color = RGB(0xFF, 0x00, 0x00); break;
		case LOG_LEVEL_FATAL:			color = RGB(0xFF, 0x00, 0x00); break;

		default:						color = RGB(0, 0, 0);
		}

		if (entry.flags & entry.FLAG_LINESTART)
		{
			char buf[LogManager::MAX_BUF_SIZE];
			size_t bufSize = sizeof(buf) - 1;

			int len = entry.formatHeader(buf, sizeof(buf));

			if (len > 0)
				_console->printString(buf, len, color);
		}

		_console->printString(entry.message, entry.messageLen, color);

		if (entry.flags & entry.FLAG_LINEEND)
			_console->printString("\n", -1, color);
	}

	virtual void onConsoleShow(ConsoleWindow* con)
	{
		_logger = new ConsoleWindowLogger(con);
		LogManager::getSingleton().attach(_logger);

		LOG(0, "[nit-rsh] usage: connect [<ip_address>]\n");
		con->setPrompt("idle> ");

		SocketBase::initialize();

		bool sendWhere = false;
		if (sendWhere)
		{
			char hostname[40];
			if (gethostname(hostname, sizeof(hostname)) != SOCKET_ERROR)
			{
				_remote.serverWhere(hostname);
			}
		}
	}

	virtual void onConsoleCommand(ConsoleWindow* con, const std::string& str)
	{ 
		if (_remote.getHostPeer() == NULL)
		{
			String cmd = str.c_str();
			if (StringUtil::startsWith(cmd, "connect"))
			{
				String addr = cmd.substr(8);
				if (_remote.connect(addr))
				{
					DataValue params = DataValue::fromJson("{ client:'nit-rsh', version:'1.0', log:true }");

					_remote.request(_remote.getHostPeer(), _channelID, RQ_ATTACH, params);
					_lastAddr = addr;
				}
				return;
			}
			else
			{
				if (!_lastAddr.empty())
				{
					if (_remote.connect(_lastAddr))
					{
						DataValue params = DataValue::fromJson("{ client:'nit-rsh', version:'1.0', log:true }");

						_remote.request(_remote.getHostPeer(), _channelID, RQ_ATTACH, params);
					}
				}

				if (_remote.getHostPeer() == NULL)
					LOG(0, "*** not connected\n");
			}
		}

		if (!str.empty())
		{
			Ref<RemotePeer> peer = _remote.getHostPeer();

			if (peer)
			{
				if (str == "bye")
					peer->disconnect();
				else
					_remote.notify(peer, _channelID, NT_COMMAND, DataValue(str.c_str(), (int) str.length()));
			}
		}
	}

	virtual void onConsoleHide(ConsoleWindow* con)
	{
		LogManager::getSingleton().detach(_logger);
	}

	virtual void onConsoleUpdate(ConsoleWindow* con)
	{
		_remote.update();
	}
};

////////////////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;

////////////////////////////////////////////////////////////////////////////////////////////

class Runtime : public nit::NitRuntime
{
public:
	Runtime()
	{
		_needDebugger = false;
	}

protected:
	virtual nit::String					getTitle()											{ return "nit-rsh"; }
	virtual bool						onInit()											{ return true; }
	virtual bool						onStart()											{ return true; }
	virtual bool						onMainLoop()										{ return true; }
	virtual int							onFinish()											{ return 0; }
	virtual void						debugCommand(const nit::String& command)			{ }
};

INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR strCmdLine, INT cmdShow)
{
	using namespace nit;

	Runtime rt;

	rt.init();

	ConsoleWindow cw;

	ConsoleApp app(&cw);

	String historyFilename;

	int numArgs;
	LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &numArgs);
	if (numArgs > 0)
	{
		String exeName;
		String exePath;
		StringUtil::splitFilename(Unicode::toUtf8(argv[0]), exeName, exePath);
		historyFilename = exePath + "nit-rsh.history.txt";
		cw.loadHistory(historyFilename.c_str());
	}

	cw.setTitle("nit remote shell");

	cw.run(hInst, hPrev, strCmdLine, cmdShow);

	if (!historyFilename.empty())
		cw.saveHistory(historyFilename.c_str());

	rt.finish();
}