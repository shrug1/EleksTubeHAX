function Rename-Bitmaps {
    param (
        [string]$directory = "."
    )

    # List all files in the directory
    $files = Get-ChildItem -Path $directory -Filter "*.bmp"

    # Extract numbers from filenames and find the highest one
    $numbers = @()
    foreach ($file in $files) {
        if ($file.Name -match "\d+") {
            $numbers += [int]$matches[0]
        }
    }
    $highestNumber = if ($numbers.Count -gt 0) { $numbers | Measure-Object -Maximum | Select-Object -ExpandProperty Maximum } else { 0 }

    # Rename files from 0.bmp to 9.bmp
    for ($i = 0; $i -lt 10; $i++) {
        $oldName = "$i.bmp"
        $oldFile = Get-ChildItem -Path $directory -Filter $oldName
        if ($oldFile) {
            $newName = "$($highestNumber + 1 + $i).bmp"
            Rename-Item -Path $oldFile.FullName -NewName (Join-Path -Path $directory -ChildPath $newName)
            Write-Output "Renamed $oldName to $newName"
        }
    }
}

# Example usage
Rename-Bitmaps -directory "."