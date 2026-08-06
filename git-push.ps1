<#
.SYNOPSIS
    One-click upload zhukong project to GitHub
    Repository: https://github.com/suqiongcxk/CXKGIT
#>

$script:ErrorActionPreference = "Continue"
Set-Location $PSScriptRoot

$REMOTE_URL = "https://github.com/suqiongcxk/CXKGIT"
$BRANCH = "main"

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  zhukong Project Upload Script" -ForegroundColor Cyan
Write-Host "  Repo: $REMOTE_URL" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# ------------------------------------------------------------
# Helper: run git, capture output, check exit code
# ------------------------------------------------------------
function Git-Cmd {
    param([string[]]$GitArgs)
    $script:ErrorActionPreference = "Continue"
    $result = & git @GitArgs 2>&1
    $ok = ($LASTEXITCODE -eq 0)
    $script:ErrorActionPreference = "Continue"
    return @{ Ok = $ok; Output = $result }
}

# ------------------------------------------------------------
# 1. Init
# ------------------------------------------------------------
if (-not (Test-Path ".git")) {
    Write-Host "[1/6] Initializing Git repo..." -ForegroundColor Yellow
    Git-Cmd "init" | Out-Null
    Git-Cmd "checkout", "-B", $BRANCH | Out-Null
    Write-Host "  [OK] Git repo initialized" -ForegroundColor Green
} else {
    Write-Host "[1/6] Git repo exists, skip init" -ForegroundColor Green
}

# ------------------------------------------------------------
# 2. Remote
# ------------------------------------------------------------
Write-Host "[2/6] Configuring remote..." -ForegroundColor Yellow
$r = Git-Cmd "remote", "get-url", "origin"
$existing = if ($r.Ok) { ($r.Output -join '').Trim() } else { $null }

if ($existing) {
    if ($existing -ne $REMOTE_URL) {
        Write-Host "  [!] Remote mismatch, updating..." -ForegroundColor Magenta
        Git-Cmd "remote", "set-url", "origin", $REMOTE_URL | Out-Null
        Write-Host "  [OK] Remote updated" -ForegroundColor Green
    } else {
        Write-Host "  [OK] Remote: $REMOTE_URL" -ForegroundColor Green
    }
} else {
    Git-Cmd "remote", "add", "origin", $REMOTE_URL | Out-Null
    Write-Host "  [OK] Remote added: $REMOTE_URL" -ForegroundColor Green
}

# ------------------------------------------------------------
# 3. Stage
# ------------------------------------------------------------
Write-Host "[3/6] Staging all files..." -ForegroundColor Yellow
Git-Cmd "add", "-A" | Out-Null
Write-Host "  [OK] All files staged" -ForegroundColor Green

# ------------------------------------------------------------
# 4. Summary
# ------------------------------------------------------------
Write-Host "[4/6] Checking changes..." -ForegroundColor Yellow
$r = Git-Cmd "status", "--porcelain"

# Filter only string lines (ignore ErrorRecord wrappers)
$lines = @()
foreach ($item in $r.Output) {
    if ($item -is [string] -and $item.Trim() -ne "") {
        $lines += $item
    }
}

$total = $lines.Count

if ($total -gt 0) {
    $staged   = ($lines | Where-Object { $_ -match '^[MADRCU]' } | Measure-Object).Count
    $unstaged = ($lines | Where-Object { $_ -match '^.[MD]' } | Measure-Object).Count
    Write-Host "  Total: $total (staged: $staged, modified: $unstaged)" -ForegroundColor White
    Write-Host ""
    Write-Host "  Changes (first 20):" -ForegroundColor Gray
    $lines | Select-Object -First 20 | ForEach-Object { Write-Host "    $_" -ForegroundColor Gray }
    if ($total -gt 20) {
        Write-Host "    ... and $($total - 20) more" -ForegroundColor Gray
    }
}

# Also check staged-only count (for first-commit scenario)
$r2 = Git-Cmd "ls-files", "--cached"
$cachedCount = 0
foreach ($item in $r2.Output) {
    if ($item -is [string] -and $item.Trim() -ne "") {
        $cachedCount++
    }
}

if ($cachedCount -eq 0) {
    Write-Host "  [X] No files staged! Check .gitignore or working directory." -ForegroundColor Red
    exit 1
}

if ($total -eq 0) {
    # No porcelain output, but files are cached (first commit scenario)
    Write-Host "  First commit: $cachedCount file(s) ready" -ForegroundColor White
} else {
    Write-Host "  Files in index: $cachedCount" -ForegroundColor Green
}

# ------------------------------------------------------------
# 5. Input commit message
# ------------------------------------------------------------
Write-Host ""
Write-Host "========================================" -ForegroundColor DarkCyan
Write-Host "  Enter commit message below:" -ForegroundColor White
Write-Host "  (Press Enter to use default: zhukong update - <timestamp>)" -ForegroundColor DarkGray
Write-Host "========================================" -ForegroundColor DarkCyan
$userMsg = Read-Host "Message"

$timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
if ($userMsg.Trim() -eq "") {
    $commitMsg = "zhukong update - $timestamp"
} else {
    $commitMsg = "$userMsg ($timestamp)"
}

# ------------------------------------------------------------
# 6. Commit
# ------------------------------------------------------------
Write-Host ""
Write-Host "[5/6] Committing: $commitMsg" -ForegroundColor Yellow
$r = Git-Cmd "commit", "-m", $commitMsg
if (-not $r.Ok) {
    Write-Host "  [FAIL] Commit failed!" -ForegroundColor Red
    $out = ($r.Output | Where-Object { $_ -is [string] }) -join "`n"
    if ($out) { Write-Host $out -ForegroundColor Red }
    exit 1
}
$out = ($r.Output | Where-Object { $_ -is [string] }) -join ' '
Write-Host "  [OK] $out" -ForegroundColor Green

# ------------------------------------------------------------
# 6. Push
# ------------------------------------------------------------
Write-Host "[6/6] Pushing to remote..." -ForegroundColor Yellow

$r = Git-Cmd "pull", "--rebase", "origin", $BRANCH
if (-not $r.Ok) {
    $msg = ($r.Output | Where-Object { $_ -is [string] }) -join ' '
    if ($msg -match "couldn.t find remote ref|no such ref|does not exist") {
        Write-Host "  [!] Empty remote, first push" -ForegroundColor Magenta
    } else {
        Write-Host "  [!] Pull note: $msg" -ForegroundColor Yellow
    }
}

$r = Git-Cmd "push", "-u", "origin", $BRANCH
if (-not $r.Ok) {
    Write-Host ""
    Write-Host "  [FAIL] Push failed!" -ForegroundColor Red
    $msg = ($r.Output | Where-Object { $_ -is [string] }) -join "`n"
    if ($msg) { Write-Host $msg -ForegroundColor Red }
    Write-Host ""
    Write-Host "  Troubleshooting:" -ForegroundColor Yellow
    Write-Host "    1. GitHub auth expired -> run: gh auth login"
    Write-Host "    2. Network issue -> check connection"
    Write-Host "    3. Conflict -> run: git pull origin $BRANCH --rebase"
    Write-Host "    4. PAT expired -> regenerate token"
    exit 1
}

Write-Host "  [OK] Push succeeded!" -ForegroundColor Green
Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  SUCCESS! Project uploaded to GitHub!" -ForegroundColor Cyan
Write-Host "  Repo: $REMOTE_URL" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "Clone command:" -ForegroundColor White
Write-Host "  git clone $REMOTE_URL" -ForegroundColor Gray