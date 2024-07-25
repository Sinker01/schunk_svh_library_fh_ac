mkdir C:\local
cd C:\local
curl -O https://archives.boost.io/release/1.82.0/source/boost_1_85_0.7z
cmd.exe /c "C:\Program Files\7-Zip\7z.exe" x boost_1_85_0.7z
cd boost_1_85_0
.\bootstrap.bat mingw
.\b2 toolset=gcc address-model=64 --with-test
