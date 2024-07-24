
mkdir C:\local
Set-Location C:\local
git clone git clone https://github.com/microsoft/vcpkg.git
Set-Location vcpkg 
.\bootstrap-vcpkg.bat
.\vcpkg install boost-test