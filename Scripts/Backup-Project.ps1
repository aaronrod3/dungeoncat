# Backup-Project.ps1
# Mirrors this project's working tree to a dedicated backup folder on another drive.
# Safety net for whatever isn't committed yet - Gitea/GitHub only capture committed history.
# Run manually anytime, or wire it to a scheduled task (see SETUP.md).
#
# Fill in the three variables below once, right after generating this repo from the template.

$SourcePath = "{{PROJECT_ROOT}}"                                  # e.g. C:\Users\you\Documents\Unreal Projects\MyGame
$DestPath   = "D:\Dev\Backups\{{PROJECT_NAME}}\Live"               # e.g. D:\Dev\Backups\MyGame\Live
$LogDir     = "D:\Dev\Backups\{{PROJECT_NAME}}\Logs"

if (-not (Test-Path (Split-Path $DestPath -Qualifier))) {
    Write-Output "$(Get-Date -Format 'yyyy-MM-dd HH:mm:ss') - backup drive not available, skipping backup."
    exit 0
}

New-Item -ItemType Directory -Force -Path $LogDir | Out-Null

# Prune logs older than 30 days so these don't accumulate forever
Get-ChildItem $LogDir -Filter "mirror_*.log" -ErrorAction SilentlyContinue |
    Where-Object { $_.LastWriteTime -lt (Get-Date).AddDays(-30) } |
    Remove-Item -Force -ErrorAction SilentlyContinue

$Timestamp = Get-Date -Format "yyyyMMdd_HHmmss"
$LogFile = Join-Path $LogDir "mirror_$Timestamp.log"

# Regeneratable build/IDE junk only - everything authored or licensed still gets mirrored.
# Add project-specific Saved/ subfolder exclusions here if you find ones that are pure
# cache/log noise (see the template's origin project for a worked example).
$ExcludeDirs = @(
    (Join-Path $SourcePath ".git"),
    (Join-Path $SourcePath "Binaries"),
    (Join-Path $SourcePath "Build"),
    (Join-Path $SourcePath "DerivedDataCache"),
    (Join-Path $SourcePath "Intermediate"),
    (Join-Path $SourcePath ".vs"),
    (Join-Path $SourcePath ".idea")
)

$StartTime = Get-Date
& robocopy.exe $SourcePath $DestPath /MIR /XD @ExcludeDirs /R:2 /W:5 /MT:8 /NP /NFL /NDL "/LOG+:$LogFile"
$ExitCode = $LASTEXITCODE
$Duration = (Get-Date) - $StartTime

# Robocopy exit codes 0-7 are all success (varying "what happened"); 8+ is a real failure.
if ($ExitCode -ge 8) {
    Write-Output "Backup FAILED (robocopy exit code $ExitCode) after $($Duration.ToString('mm\:ss')). See $LogFile"
    exit 1
} else {
    Write-Output "Backup completed (robocopy exit code $ExitCode) in $($Duration.ToString('mm\:ss')). Log: $LogFile"
    exit 0
}
