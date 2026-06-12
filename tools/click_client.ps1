# Clicks at a client-area coordinate of a process's main window (DPI-aware).
param(
    [Parameter(Mandatory=$true)][string]$ProcName,
    [Parameter(Mandatory=$true)][int]$X,
    [Parameter(Mandatory=$true)][int]$Y
)

Add-Type @"
using System;
using System.Runtime.InteropServices;
public class WinClick {
  [DllImport("user32.dll")] public static extern bool SetProcessDPIAware();
  [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr h);
  [DllImport("user32.dll")] public static extern bool ClientToScreen(IntPtr h, ref POINT p);
  [DllImport("user32.dll")] public static extern bool SetCursorPos(int x, int y);
  [DllImport("user32.dll")] public static extern void mouse_event(uint f, uint dx, uint dy, uint d, IntPtr e);
  public struct POINT { public int X, Y; }
}
"@

[WinClick]::SetProcessDPIAware() | Out-Null
$p = Get-Process -Name $ProcName -ErrorAction SilentlyContinue | Where-Object { $_.MainWindowHandle -ne 0 } | Select-Object -First 1
if (-not $p) { Write-Output "ERROR: process '$ProcName' not found"; exit 1 }
$h = $p.MainWindowHandle

[WinClick]::SetForegroundWindow($h) | Out-Null
Start-Sleep -Milliseconds 300

$pt = New-Object WinClick+POINT
$pt.X = $X; $pt.Y = $Y
[WinClick]::ClientToScreen($h, [ref]$pt) | Out-Null
[WinClick]::SetCursorPos($pt.X, $pt.Y) | Out-Null
Start-Sleep -Milliseconds 120
[WinClick]::mouse_event(0x02, 0, 0, 0, [IntPtr]::Zero)  # LEFTDOWN
Start-Sleep -Milliseconds 60
[WinClick]::mouse_event(0x04, 0, 0, 0, [IntPtr]::Zero)  # LEFTUP
Start-Sleep -Milliseconds 150
Write-Output "clicked client ($X,$Y) -> screen ($($pt.X),$($pt.Y))"
