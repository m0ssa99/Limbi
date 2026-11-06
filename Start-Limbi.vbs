Set shell = CreateObject("WScript.Shell")
Set fs = CreateObject("Scripting.FileSystemObject")
script = fs.BuildPath(fs.GetParentFolderName(WScript.ScriptFullName), "Limbi.ps1")
extra = ""
If WScript.Arguments.Count > 0 Then extra = " -Tray"
shell.Run "powershell.exe -NoProfile -STA -ExecutionPolicy Bypass -File """ & script & """" & extra, 0, False
