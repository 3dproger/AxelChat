# AxelChat
A program for adding interactivity to live broadcasts and communication with viewers

<p align="center">
  <img src="misc/github-social4.png">
</p>

<b><a href="/../../releases">Download</a></b>

# Screenshots
<p align="center">
  <img src="misc/screenshots/2.png">
  <img src="misc/screenshots/5.png">
  <img src="misc/screenshots/6.png">
  <img src="misc/screenshots/7.png">
  
  <img src="misc/screenshots/3.png">
  <img src="misc/screenshots/4.png">
</p>

# Building for Windows x64
1. Clone repo or download source
2. Install Visual Studio Community 2019: https://visualstudio.microsoft.com
  <br> 3.1 In Visual Studio Installer enable "Development of classic C ++ applications" component
  <br> 3.2 In tab "Language Packs" enable only English beacuse there may be encoding problems in displaying compiler messages when in other languages
  <br> 3.3 After installation, you do not need to run Visual Studio
3. Download Open Source Qt online installer (sign-up or login required during installing): https://www.qt.io/download-qt-installer or you can find the offline installers: https://www.qt.io/offline-installers
4. Run and login and install Qt 5.15.x or 5.12.x (for online installer):
  <br> 4.1. In page "Qt Open Source Usage Obligations" check "I have read and approve the obligations of using Open Source Qt"
  <br> 4.2. In page "Selection of Components" you need enabled components:
    * Qt / Qt 5.15.x / MSVC 2019 64-bit
    * Developer and Designer Tools / Qt Creator x.x.x CDB Debugger Support
    * Developer and Designer Tools / Debugging Tools for Windows
    * Developer and Designer Tools / OpenSSL 1.1.1x Toolkit / OpenSSL 32-bit binaries
    * Developer and Designer Tools / OpenSSL 1.1.1x Toolkit / OpenSSL 64-bit binaries

5. ...
