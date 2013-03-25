import nit

var fl = FileLocator("$root", "c:/proj/nit", true, true)
var files = fl.Find("*.cpp")
files.extend(fl.Find("*.c"))
files.extend(fl.Find("*.h"))
files.extend(fl.Find("*.mm"))
files.extend(fl.Find("*.m"))
files.extend(fl.Find("*.inl"))
files.extend(fl.Find("*.nit"))
files.extend(fl.Find("*.cfg"))

files.sort(@(a,b) => a.Name <=> b.Name)

var file = fl.Locate("src/nit2d/CocosService.cpp")

function findNonAsciiLine(file: File)
{
  var reader
  try reader = MemoryBuffer.Reader(file.Open()) 
  catch(ex) { stdout.printf("*** can't open: %s: %s", file.URL, ex); return null }

  var lines = []
  var line = 1
  var skip = false

  var ch

  if (!reader.Eof)
  {
    // skip BOM
    ch = reader.ReadU8()
    if (ch == 0xEF) 
    {
      if (reader.ReadU8() != 0xBB && !skip) { skip = true; lines.push(line) }
      if (reader.ReadU8() != 0xBF && !skip) { skip = true; lines.push(line) }
    }
  }
  
  while (!reader.Eof)
  {
    ch = reader.ReadU8()

    if (ch == char('\n')) { ++line; skip = false; }

    if (!skip && ch > 127)  { skip = true; lines.push(line) }
  }

  return lines.len() ? lines : null
}

foreach (file in files)
{
  var lines = findNonAsciiLine(file)
  if (lines != null) 
  { 
    lines.reverse()
    foreach (line in lines)
      stdout.printf("%s at line %d", file.URL, line); 
  }
}
