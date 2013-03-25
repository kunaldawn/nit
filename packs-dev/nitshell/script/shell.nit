import nit

var p = CmdLineParser()

p.addSection(app.settings.getSection("shell"))

var ss = Settings()
var error

try 
	p.parse(ss, app.runtime.arguments, true, 2)
catch (ex)
	error = ex.tostring()

var files = ss.find("file")
var lines = ss.find("line")
var help = ss.get("help", "", false) == "true"
var running = ss.get("running", "", false) == "true"
var quiet = ss.get("quiet", "", false) == "true"
var std = ss.get("std", "true", false) == "true"

if (quiet)
{
	::print := @(...) {}
	::printf := @(...) {}
	::error := @(...) {}
}
else if (std)
{
	::print := stdout.print.bindenv(stdout)
	::printf := stdout.printf.bindenv(stdout)
	::error := stderr.print.bindenv(stderr)
}

if (error || help || (lines.len() == 0 && files.len() == 0 && !running))
{
	if (error)
		error = "*** " + error
	p.showUsage("nit script", "", error)
	app.stop(error ? -1 : 0)
}

if (!running)
	app.stop(0)

var workDir = FileLocator("$work", app.getConfig("win32/work_path", "."))

session.package.require(workDir)

foreach (f in files)
	dofile(f)

foreach (l in lines)
	compilestring(l)()