$sourceExe = "NavyEngine\x64\Release\NavyEngine.exe"
$sourceDlls = "NavyEngine\x64\Release\*.dll"
$assetsDir = "NavyEngine\NavyEngine"
$releaseFolder = "RecorridoVirtual_Windows_x64"
$zipFile = "RecorridoVirtual_Windows_x64.zip"

Write-Host "Iniciando el empaquetado del proyecto..."

# 1. Crear carpeta limpia
if (Test-Path $releaseFolder) {
    Remove-Item -Recurse -Force $releaseFolder
}
New-Item -ItemType Directory -Force -Path $releaseFolder | Out-Null

# 2. Copiar Ejecutable y DLLs
Write-Host "Copiando binarios..."
Copy-Item -Path $sourceExe -Destination $releaseFolder\
Copy-Item -Path $sourceDlls -Destination $releaseFolder\

# 3. Copiar Carpetas de Recursos (Assets)
Write-Host "Copiando recursos (Modelos, Audio, UI, HDR)..."
Copy-Item -Path "$assetsDir\Audio" -Destination $releaseFolder\ -Recurse
Copy-Item -Path "$assetsDir\HDR" -Destination $releaseFolder\ -Recurse
Copy-Item -Path "$assetsDir\Modelos3D" -Destination $releaseFolder\ -Recurse
Copy-Item -Path "$assetsDir\UI" -Destination $releaseFolder\ -Recurse

# 4. Copiar Shaders
Write-Host "Copiando shaders..."
Copy-Item -Path "$assetsDir\*.vs" -Destination $releaseFolder\
Copy-Item -Path "$assetsDir\*.fs" -Destination $releaseFolder\
Copy-Item -Path "$assetsDir\*.glsl" -Destination $releaseFolder\

# 5. Comprimir a ZIP
Write-Host "Comprimiendo en un archivo ZIP..."
if (Test-Path $zipFile) {
    Remove-Item -Force $zipFile
}
Compress-Archive -Path "$releaseFolder\*" -DestinationPath $zipFile

Write-Host "¡Empaquetado exitoso! Archivo creado: $zipFile"
