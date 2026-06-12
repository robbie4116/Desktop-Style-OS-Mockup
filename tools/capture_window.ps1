# Captures the client area of a process's main window to a PNG file.
# Brings the window to the foreground first; optionally maximizes it.
param(
    [Parameter(Mandatory=$true)][string]$ProcName,
    [Parameter(Mandatory=$true)][string]$Out,
    [switch]$Maximize
)

Add-Type @"
using System;
using System.Runtime.InteropServices;
public class WinCap {
  [DllImport("user32.dll")] public static extern bool SetProcessDPIAware();
  [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr h);
  [DllImport("user32.dll")] public static extern bool ShowWindow(IntPtr h, int n);
  [DllImport("user32.dll")] public static extern bool GetClientRect(IntPtr h, out RECT r);
  [DllImport("user32.dll")] public static extern bool ClientToScreen(IntPtr h, ref POINT p);
  public struct RECT { public int Left, Top, Right, Bottom; }
  public struct POINT { public int X, Y; }
}
"@

[WinCap]::SetProcessDPIAware() | Out-Null

$p = Get-Process -Name $ProcName -ErrorAction SilentlyContinue | Where-Object { $_.MainWindowHandle -ne 0 } | Select-Object -First 1
if (-not $p) { Write-Output "ERROR: process '$ProcName' (with a window) not found"; exit 1 }
$h = $p.MainWindowHandle

if ($Maximize) { [WinCap]::ShowWindow($h, 3) | Out-Null; Start-Sleep -Milliseconds 400 }
[WinCap]::SetForegroundWindow($h) | Out-Null
Start-Sleep -Milliseconds 400

$cr = New-Object WinCap+RECT
[WinCap]::GetClientRect($h, [ref]$cr) | Out-Null
$tl = New-Object WinCap+POINT
$tl.X = 0; $tl.Y = 0
[WinCap]::ClientToScreen($h, [ref]$tl) | Out-Null
$w = $cr.Right - $cr.Left
$ht = $cr.Bottom - $cr.Top
if ($w -le 0 -or $ht -le 0) { Write-Output "ERROR: bad client size $w x $ht"; exit 1 }

Add-Type -AssemblyName System.Drawing
$bmp = New-Object System.Drawing.Bitmap $w, $ht
$g = [System.Drawing.Graphics]::FromImage($bmp)
$g.CopyFromScreen($tl.X, $tl.Y, 0, 0, (New-Object System.Drawing.Size($w, $ht)))
$bmp.Save($Out, [System.Drawing.Imaging.ImageFormat]::Png)
$g.Dispose(); $bmp.Dispose()
Write-Output "saved $Out ($w x $ht)"
