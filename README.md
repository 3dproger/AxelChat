# AxelChat
A program for adding interactivity to live broadcasts and communication with viewers

<p align="center">
  <img src="misc/github-social4.png">
</p>

<b><a href="/../../releases">Download</a></b>

# Build (deprecated)
### Setting Up the Environment for Windows 7 and newer, x64
1. Download Open Source Qt online installer (sign-up or login required during installing): https://www.qt.io/download-qt-installer or you can also find the offline installers but this is not the recommended way
2. Run Qt installer and login/sign-up
3. While installing select components:
    * Qt / Qt 5.15.x / MinGW x.x.x 64-bit
    * Developer and Designer Tools / Qt Creator x.x.x CDB Debugger Support
    * Developer and Designer Tools / Debugging Tools for Windows
    * Developer and Designer Tools / OpenSSL 1.1.1x Toolkit / OpenSSL 32-bit binaries
    * Developer and Designer Tools / OpenSSL 1.1.1x Toolkit / OpenSSL 64-bit binaries
### Build source
1. Clone repo or download source
2. Open file ```<AxelChat_project_folder>/src/AxelChat.pro``` with Qt Creator
3. In Qt Creator select ```Desktop Qt x.x.x MinGW 64-bit``` kit
4. Select ```Release``` building mode
5. Click button ```Build Project "AxelChat"```. If successful build, folder ```release_win64``` will appear in the project folder with AxelChat.exe and other files
6. Copy files from ```<AxelChat_project_folder>/app_files/win64``` to ```<AxelChat_project_folder>/release_win64```
7. Run ```<AxelChat_project_folder>/release_win64/AxelChat.exe``` or click ```Run``` in Qt Creator
8. For subsequent builds, just close ```AxelChat.exe``` application and click ```Run``` in Qt Creator
9. To release an AxelChat for use by other users, check if all the required DLLs (\*.os for Linux) are in the folder except system DLLs (\*.os for Linux). Utilities https://github.com/3dproger/ProcDepMonitor or https://www.dependencywalker.com/ or analogs can helps you

# Screenshots
<p align="center">
  <img src="misc/screenshots/2.png">
  <img src="misc/screenshots/5.png">
  <img src="misc/screenshots/6.png">
  <img src="misc/screenshots/7.png">
  
  <img src="misc/screenshots/3.png">
  <img src="misc/screenshots/4.png">
</p>
