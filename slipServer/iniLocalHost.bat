pushd .
cd %~dp0
set xamp=f:\xampp
"d:\program files\7-Zip\7z.exe" x -of:\ d:\www\webserver\xampp-portable-windows-x64-8.2.12-0-VS16.7z
set tgt=%xamp%\htdocs\
copy language.php%1      %tgt%\language.php
copy login.php%1         %tgt%\index.php
copy login.php%1         %tgt%\login.php
copy nextSlip.php%1      %tgt%\nextSlip.php
copy slip.php%1          %tgt%\slip.php
copy slip.js%1           %tgt%\slip.js
copy slip.css%1          %tgt%\slip.css
copy calcScore.js%1      %tgt%\calcScore.js
copy ready.php%1         %tgt%\ready.php
copy bridge.jpg          %tgt%\bridge.jpg

popd
cd /d %xamp%
call setup_xampp.bat
rem conf is set to local ip, so can be reached within local network
rem copy d:httpd.conf %xamp%\apache\conf\
call apache_start.bat
