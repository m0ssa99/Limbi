Add-Type -AssemblyName System.Windows.Forms
Add-Type -AssemblyName System.Drawing
[System.Windows.Forms.Application]::EnableVisualStyles()
$mutex = New-Object System.Threading.Mutex($false, 'Local\LimbiPrototype')
if (-not $mutex.WaitOne(0)) { exit }
$dataDir = Join-Path $env:LOCALAPPDATA 'LimbiPrototype'
[void][IO.Directory]::CreateDirectory($dataDir)
$statePath = Join-Path $dataDir 'state.json'
$script:state = @{ Random = $true; Minutes = 60; Start = 9; End = 20; Saved = @(); Known = @(); Recent = @() }
if (Test-Path $statePath) {
    try { $loaded = Get-Content $statePath -Raw -Encoding UTF8 | ConvertFrom-Json; foreach ($p in $loaded.PSObject.Properties) { $script:state[$p.Name] = $p.Value } } catch { }
}
function Save-State { $script:state | ConvertTo-Json -Depth 5 | Set-Content $statePath -Encoding UTF8 }
$script:entries = @(Get-Content (Join-Path $PSScriptRoot 'content.json') -Raw -Encoding UTF8 | ConvertFrom-Json)
$script:current = $null
$script:paused = $false
$script:pauseDate = (Get-Date).Date
function Set-Next {
    $minutes = [int]$script:state.Minutes
    if ($script:state.Random) { $minutes = Get-Random -Minimum ([Math]::Max(1,$minutes-15)) -Maximum ($minutes+16) }
    $script:next = (Get-Date).AddMinutes($minutes)
}
function Pick-Entry {
    $pool = @($script:entries | Where-Object { $_.id -notin @($script:state.Known) -and $_.id -notin @($script:state.Recent) })
    if (!$pool.Count) { $pool = @($script:entries | Where-Object { $_.id -notin @($script:state.Known) }) }
    if (!$pool.Count) { $pool = $script:entries }
    $script:current = $pool | Get-Random
    $script:state.Recent = @(@($script:state.Recent) + $script:current.id | Select-Object -Last 5)
    Save-State
}
$form = New-Object Windows.Forms.Form
$form.Text = 'Limbi - expresia zilei'
$form.Size = New-Object Drawing.Size(740,650)
$form.MinimumSize = New-Object Drawing.Size(650,580)
$form.StartPosition = 'CenterScreen'
$form.BackColor = [Drawing.Color]::FromArgb(245,247,252)
$form.Font = New-Object Drawing.Font('Segoe UI',11)
$layout = New-Object Windows.Forms.TableLayoutPanel
$layout.Dock = 'Fill'; $layout.Padding = New-Object Windows.Forms.Padding(24)
$layout.ColumnCount = 1; $layout.RowCount = 4
[void]$layout.RowStyles.Add((New-Object Windows.Forms.RowStyle('Absolute',45)))
[void]$layout.RowStyles.Add((New-Object Windows.Forms.RowStyle('Percent',100)))
[void]$layout.RowStyles.Add((New-Object Windows.Forms.RowStyle('Absolute',55)))
[void]$layout.RowStyles.Add((New-Object Windows.Forms.RowStyle('Absolute',40)))
$title = New-Object Windows.Forms.Label
$title.Text = 'Un pas mic, in trei limbi'; $title.AutoSize = $true
$title.Font = New-Object Drawing.Font('Segoe UI',18,[Drawing.FontStyle]::Bold)
$body = New-Object Windows.Forms.RichTextBox
$body.Dock = 'Fill'; $body.ReadOnly = $true; $body.BorderStyle = 'None'
$body.BackColor = [Drawing.Color]::White; $body.Font = New-Object Drawing.Font('Segoe UI',12)
$buttons = New-Object Windows.Forms.FlowLayoutPanel; $buttons.Dock = 'Fill'
$status = New-Object Windows.Forms.Label; $status.Dock = 'Fill'
function Refresh-Card {
    if (!$script:current) { Pick-Entry }
    $e = $script:current
    $body.Text = "ENGLEZA`n$($e.en)`n`nROMANA`n$($e.ro)`n`nITALIANA`n$($e.it)`n`nSENS`n$($e.meaning)`n`nEXEMPLU`n$($e.exampleEn)`n$($e.exampleRo)`n$($e.exampleIt)"
    $status.Text = "Salvate: $(@($script:state.Saved).Count) | Invatate: $(@($script:state.Known).Count) | $($e.kind)"
}
function Show-Card { Refresh-Card; $form.Show(); $form.WindowState = 'Normal'; $form.Activate() }
function Add-Button($text, $action) {
    $b = New-Object Windows.Forms.Button; $b.Text = $text; $b.AutoSize = $true; $b.Height = 36
    $b.Add_Click($action); [void]$buttons.Controls.Add($b)
}
Add-Button 'Urmatoarea' { Pick-Entry; Refresh-Card }
Add-Button 'Salveaza' { $script:state.Saved = @(@($script:state.Saved) + $script:current.id | Select-Object -Unique); Save-State; Refresh-Card }
Add-Button 'Stiu deja' { $script:state.Known = @(@($script:state.Known) + $script:current.id | Select-Object -Unique); Save-State; Pick-Entry; Refresh-Card }
Add-Button 'Mai repeta' { $script:next = (Get-Date).AddMinutes(10); $form.Hide() }
[void]$layout.Controls.Add($title,0,0); [void]$layout.Controls.Add($body,0,1)
[void]$layout.Controls.Add($buttons,0,2); [void]$layout.Controls.Add($status,0,3)
$form.Controls.Add($layout)
$tray = New-Object Windows.Forms.NotifyIcon
$tray.Icon = [Drawing.SystemIcons]::Information; $tray.Text = 'Limbi'; $tray.Visible = $true
$menu = New-Object Windows.Forms.ContextMenuStrip
function Menu-Item($text,$action) { $item = New-Object Windows.Forms.ToolStripMenuItem($text); $item.Add_Click($action); [void]$menu.Items.Add($item); return $item }
$null = Menu-Item 'Arata expresia curenta' { Show-Card }
function Notify-New {
    Pick-Entry
    $tray.ShowBalloonTip(10000,'Limbi - o expresie noua','Apasa aici pentru traduceri, sens si un exemplu.',[Windows.Forms.ToolTipIcon]::Info)
}
$null = Menu-Item 'Testeaza notificarea' { Notify-New }
$null = Menu-Item 'Expresii salvate' {
    $saved = @($script:entries | Where-Object { $_.id -in @($script:state.Saved) })
    $text = ($saved | ForEach-Object { "$($_.en) / $($_.ro) / $($_.it)" }) -join "`n`n"
    if (!$text) { $text = 'Nu ai salvat inca expresii.' }
    [void][Windows.Forms.MessageBox]::Show($text,'Expresii salvate')
}
$null = Menu-Item 'Pauza / reia pentru azi' { $script:paused = !$script:paused; $script:pauseDate = (Get-Date).Date; Set-Next; $tray.Text = if ($script:paused) { 'Limbi - pauza' } else { 'Limbi' } }
$startupPath = Join-Path ([Environment]::GetFolderPath('Startup')) 'Limbi.lnk'
$null = Menu-Item 'Setari' {
    $dialog = New-Object Windows.Forms.Form; $dialog.Text = 'Setari Limbi'; $dialog.Size = New-Object Drawing.Size(390,360); $dialog.StartPosition = 'CenterParent'
    $panel = New-Object Windows.Forms.FlowLayoutPanel; $panel.Dock = 'Fill'; $panel.FlowDirection = 'TopDown'; $panel.Padding = New-Object Windows.Forms.Padding(15)
    $fields = @{}
    foreach ($spec in @(@('Minutes','Interval (minute)',15,240),@('Start','Ora inceperii',0,23),@('End','Ora terminarii',1,24))) {
        $label = New-Object Windows.Forms.Label; $label.Text = $spec[1]; $label.AutoSize = $true; $panel.Controls.Add($label)
        $input = New-Object Windows.Forms.NumericUpDown; $input.Minimum = $spec[2]; $input.Maximum = $spec[3]; $input.Value = $script:state[$spec[0]]; $panel.Controls.Add($input); $fields[$spec[0]] = $input
    }
    $random = New-Object Windows.Forms.CheckBox; $random.Text = 'Aleatoriu (+/- 15 minute)'; $random.Width = 300; $random.Checked = $script:state.Random; $panel.Controls.Add($random)
    $startup = New-Object Windows.Forms.CheckBox; $startup.Text = 'Porneste cu Windows'; $startup.Width = 300; $startup.Checked = Test-Path $startupPath; $panel.Controls.Add($startup)
    $save = New-Object Windows.Forms.Button; $save.Text = 'Salveaza'; $panel.Controls.Add($save)
    $save.Add_Click({
        if ($fields.Start.Value -ge $fields.End.Value) { [void][Windows.Forms.MessageBox]::Show('Ora inceperii trebuie sa fie inaintea terminarii.'); return }
        foreach ($key in $fields.Keys) { $script:state[$key] = [int]$fields[$key].Value }; $script:state.Random = $random.Checked
        if ($startup.Checked) {
            $shell = New-Object -ComObject WScript.Shell; $link = $shell.CreateShortcut($startupPath)
            $link.TargetPath = Join-Path $env:WINDIR 'System32\wscript.exe'; $link.Arguments = '"' + (Join-Path $PSScriptRoot 'Start-Limbi.vbs') + '" -Tray'; $link.WorkingDirectory = $PSScriptRoot; $link.Save()
        } elseif (Test-Path $startupPath) { Remove-Item -LiteralPath $startupPath }
        Save-State; Set-Next; $dialog.Close()
    })
    $dialog.Controls.Add($panel); [void]$dialog.ShowDialog($form); $dialog.Dispose()
}
$null = Menu-Item 'Inchide aplicatia' { $script:exiting = $true; $form.Close(); [Windows.Forms.Application]::ExitThread() }
$tray.ContextMenuStrip = $menu
$tray.Add_DoubleClick({ Show-Card }); $tray.Add_BalloonTipClicked({ Show-Card })
$form.Add_FormClosing({ param($sender,$eventArgs) if (!$script:exiting -and $eventArgs.CloseReason -eq 'UserClosing') { $eventArgs.Cancel = $true; $form.Hide() } })
$timer = New-Object Windows.Forms.Timer; $timer.Interval = 15000
$timer.Add_Tick({
    $now = Get-Date
    if ($now.Date -ne $script:pauseDate) { $script:paused = $false; $script:pauseDate = $now.Date; $tray.Text = 'Limbi' }
    if (!$script:paused -and $now.Hour -ge $script:state.Start -and $now.Hour -lt $script:state.End -and $now -ge $script:next) { Notify-New; Set-Next }
})
Set-Next; $timer.Start()
try {
    # Pornirea automata ramane discreta; lansarea manuala arata cardul.
    $form.Add_Shown({ $form.Hide() })
    if ($args -notcontains '-Tray') { Refresh-Card; $form.Add_Shown({ $form.Show(); $form.Activate() }) }
    [Windows.Forms.Application]::Run($form)
} finally { $timer.Dispose(); $tray.Dispose(); $mutex.ReleaseMutex(); $mutex.Dispose() }
