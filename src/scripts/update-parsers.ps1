# Copyright 2013-2020 Yikun Liu <cos.lyk@gmail.com>
#
# This program is free software: you can redistribute it
# and/or modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation, either version 3 of the
# License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be
# useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
# Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program. If not, see http://www.gnu.org/licenses/.


# Define functions to check version
function Get-Latest-Version-Github {
    param (
        $repo
    )
    $url = "https://api.github.com/repos/$repo/releases/latest"
    try {
        $response = Invoke-WebRequest $url -ErrorAction Stop
        return (ConvertFrom-Json -InputObject $response).tag_name
    }
    catch {
        Write-Output "Cannot get the latest version."
        Exit
    }
}

function Get-Current-Version {
    param (
        $plugin_name
    )
    $path = "$env:LOCALAPPDATA\MoonPlayer\version-$plugin_name.txt"
    if (Test-Path $path) {
        return Get-Content -Path "$path"
    } else {
        return "Not installed"
    }
}

function Save-Version-Info {
    param (
        $plugin_name,
        $version
    )
    $version > "$env:LOCALAPPDATA\MoonPlayer\version-$plugin_name.txt"
}

### Set Github Mirror
if ((Get-TimeZone).Id -eq "China Standard Time") {
    $github_mirror = "https://download.fastgit.org"
} else {
    $github_mirror = "https://github.com"
}


### Update lux
Write-Output "-------- Checking lux's updates -------"

# Get current lux version
$current_version = Get-Current-Version "lux"
Write-Output "Current version: $current_version"

# Get latest lux version
$latest_version = Get-Latest-Version-Github "iawia002/lux"
Write-Output "Latest version: $latest_version"

# Check if the version is latest
if ($latest_version -eq $current_version) {
    Write-Output "Lux is already up-to-date."
} else {
    Write-Output ""
    Write-Output "------------ Updating lux -------------"

    # Download
    Write-Output "Downloading latest version..."
    $version_no_v = $latest_version.Substring(1)
    $url = "$github_mirror/iawia002/lux/releases/download/$latest_version/lux_${version_no_v}_Windows_64-bit.zip"
    Write-Output $url
    $output = "$env:LOCALAPPDATA\MoonPlayer\lux.zip"
    (New-Object System.Net.WebClient).DownloadFile($url, $output)

    # Extract
    Write-Output "Extracting lux..."
    Expand-Archive "$output" -DestinationPath "$env:LOCALAPPDATA\MoonPlayer" -Force
    Save-Version-Info "lux" $latest_version
}


### Update yt-dlp
Write-Output "-------- Checking yt-dlp's updates -------"

# Get latest yt-dlp version
$latest_version = Get-Latest-Version-Github "yt-dlp/yt-dlp"
Write-Output "Latest version: $latest_version"

# Get current yt-dlp version
$current_version = Get-Current-Version "yt-dlp"
Write-Output "Current version: $current_version"

# Check if the version is latest
if ($latest_version -eq $current_version) {
    Write-Output "Yt-dlp already up-to-date."
} else {
    Write-Output ""
    Write-Output "------------ Updating yt-dlp -------------"
    Write-Output "Downloading latest version..."
    $url = "$github_mirror/yt-dlp/yt-dlp/releases/download/$latest_version/yt-dlp.exe"
    Write-Output $url
    $output = "$env:LOCALAPPDATA\MoonPlayer\yt-dlp.exe"
    (New-Object System.Net.WebClient).DownloadFile($url, $output)
    Save-Version-Info "yt-dlp" $latest_version
}



### Update plugins
Write-Output ""
Write-Output "---------- Checking plugins' updates ---------"

# Get current plugins version
$current_version = Get-Current-Version "plugins"
Write-Output "Current version: $current_version"

# Get latest plugins version
$latest_version = Get-Latest-Version-Github "coslyk/moonplayer-plugins"
Write-Output "Latest version: $latest_version"

# Check if the version is latest
if ($latest_version -eq $current_version) {
    Write-Output "Plugins are already up-to-date."
} else {
    Write-Output "-------------- Updating plugins --------------"

    # Download
    Write-Output "Downloading plugins..."
    $url = "$github_mirror/coslyk/moonplayer-plugins/releases/download/$latest_version/plugins.zip"
    Write-Output $url
    $output = "$env:LOCALAPPDATA\MoonPlayer\plugins.zip"
    (New-Object System.Net.WebClient).DownloadFile($url, $output)

    # Extract
    Write-Output "Extracting plugins..."
    Expand-Archive "$output" -DestinationPath "$env:LOCALAPPDATA\MoonPlayer\plugins" -Force
    Save-Version-Info "plugins" $latest_version
    Write-Output "Finished. You need to restart MoonPlayer to load plugins."
}
