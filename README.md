[Download](https://github.com/3dproger/axelchat/releases/latest/) |
[Releases](https://github.com/3dproger/AxelChat/releases) |
[Issues](https://github.com/3dproger/AxelChat/issues) |
[Patreon](https://patreon.com/axel_k)

[<img src="images/button-download.png">](https://github.com/3dproger/AxelChat/releases)
[![Patreon](https://raw.githubusercontent.com/3dproger/AxelChat/main/images/button-patron.png)](https://patreon.com/axel_k)

<p align="center">
  <img src="images/github-social5b.png">
</p>

# Features
- Display messages from multiple streaming platforms at once. For example YouTube and Twitch
- Display messages from any live YouTube chat without authorization. Messages in AxelChat arrive much faster than the standard YouTube live chat in your browser
- Display messages from any live Twitch chat with avatars of participants
- Displaying the number of viewers both total and for individual platforms
- Displays detailed information about a participant in a live chat in a separate window (including a link to the participant's channel)
<p align="center">
  <img src="images/9.png">
</p>

- Sound commands that are activated by the corresponding message from the chat. There are built-in commands, but you can always add your own
- Saving messages and other information on the fly to text files. It may be useful for further processing by other software
- Proxy support (experimental)
- Link display for quick and easy broadcast management

# Possibly in future updates
- Support for other live streaming platforms
- Support for donate platforms
- Support for other operating systems, including mobile
- Many other improvements

# FAQ
- Is AxelChat considered a viewer? - No, at least for now, for YouTube and Twitch
- Whether AxelChat sends any user data? - No, except for what is required for authorization on Twitch
- Does AxelChat work with YouTube premieres? - Yes
- Can I download a YouTube chat for an already past stream using the AxelChat? - No, use something else instead. For example, use the browser extension [Save Live Streaming Chats for YouTube™](https://chrome.google.com/webstore/detail/save-live-streaming-chats/bcclhcedlelimlnmcpfeiabljbpahnef)
- Is the source code hidden? - Yes. Until recently, the source code was open, but I hid it due to the reluctance to leak my "company secrets" to potential competitors. I do not exclude that someday I will open the source code again

# Known Issues
- YouTube: messages that YouTube may find inappropriate are not displayed
- YouTube: works only with open access broadcasts or available via the link. Private broadcasts are not supported
- YouTube: Doesn't work for streams with age and possibly other restrictions

# Troubleshooting
### Doesn't connect (General)
- Check your internet connection
- Check proxy settings if using
- Check your firewall settings

### Doesn't connect (YouTube)
- Check that the link to the broadcast is correct
- Check that the broadcast is incomplete
- Check that the broadcast is publicly available or accessed via a link. Private broadcasts are not supported

### Doesn't connect (Twitch)
- Check that the link to the channel is correct
- Check that the OAuth-token is correct. Update the OAuth-token, if necessary, by pressing the corresponding button in the AxelChat
- Check in the settings of your Twitch account if the permission to use the twitch account has not been revoked. Update the OAuth-token, if necessary, by pressing the corresponding button in the AxelChat

# Build AxelChat from source for Windows 7 and newer, x64
## Installing Qt
1. Download [Open Source Qt online installer](https://www.qt.io/download-qt-installer) or you can also find the offline installers but this is not the recommended way
2. Run Qt installer and login/sign-up if required
3. While installing select components:
    * Qt / Qt 5.15.x / MinGW x.x.x 64-bit
    * Developer and Designer Tools / Qt Creator x.x.x CDB Debugger Support
    * Developer and Designer Tools / Debugging Tools for Windows
    * Developer and Designer Tools / OpenSSL 1.1.1x Toolkit / OpenSSL 32-bit binaries
    * Developer and Designer Tools / OpenSSL 1.1.1x Toolkit / OpenSSL 64-bit binaries

## Build source
1. Clone repo or download source
2. Open file ```<AxelChat_project_folder>/src/AxelChat.pro``` with Qt Creator
3. In Qt Creator select ```Desktop Qt x.x.x MinGW 64-bit``` kit
4. Select ```Release``` building mode
5. Click button ```Build Project "AxelChat"```. If successful build, folder ```release_win64``` will appear in the project folder with AxelChat.exe and other files
6. Copy files from ```<AxelChat_project_folder>/app_files/win64``` to ```<AxelChat_project_folder>/release_win64```
7. Run ```<AxelChat_project_folder>/release_win64/AxelChat.exe``` or click ```Run``` in Qt Creator
8. For subsequent builds, just close ```AxelChat.exe``` application and click ```Run``` in Qt Creator

Note: to release an AxelChat for use by other users, I recommend to check if all the required DLLs (\*.so for Linux) are in the folder. [ProcDepMonitor](https://github.com/3dproger/ProcDepMonitor) or [DependencyWalker](https://www.dependencywalker.com/) or analogs can helps you
