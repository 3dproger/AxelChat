# AxelChat <img width="32" height="32" src="src/resources/images/youtube-icon.svg"> <img width="32" height="32" src="src/resources/images/twitch-icon.svg"> <img width="32" height="32" src="src/resources/images/trovo-icon.svg"> <img width="32" height="32" src="src/resources/images/kick-icon.svg"> <img width="32" height="32" src="src/resources/images/rumble-icon.svg"> <img width="32" height="32" src="src/resources/images/odysee-icon.svg"> <img width="32" height="32" src="src/resources/images/goodgame-icon.svg"> <img width="32" height="32" src="src/resources/images/vkplaylive-icon.svg"> <img width="32" height="32" src="src/resources/images/vkvideo-icon.svg"> <img width="32" height="32" src="src/resources/images/wasd-icon.svg"> <img width="32" height="32" src="src/resources/images/telegram-icon.svg"> <img width="32" height="32" src="src/resources/images/discord-icon.svg"> <img width="32" height="32" src="src/resources/images/telegram-icon.svg"> <img width="32" height="32" src="src/resources/images/donationalerts-icon.svg">
The AxelChat application collects chat from various streaming platforms and displays it in one window. Has a user-friendly interface and various useful features

[Download](https://github.com/3dproger/axelchat/releases/latest/) |
[Releases](https://github.com/3dproger/AxelChat/releases) |
[Issues](https://github.com/3dproger/AxelChat/issues) |
[Sponsor](https://3dproger.github.io/AxelChat/sponsor) |
[Privacy Policy](https://3dproger.github.io/AxelChat/privacy) |
[License, Terms and Conditions](https://github.com/3dproger/AxelChat/blob/main/LICENSE)

[<img src="misc/images/button-download.png">](https://github.com/3dproger/AxelChat/releases)
[<img src="misc/images/button-support.png">](https://3dproger.github.io/AxelChat/sponsor)

[![Download AxelChat](https://a.fsdn.com/con/app/sf-download-button)](https://sourceforge.net/projects/axelchat/files/latest/download)

<p align="center">
  <img src="misc/images/github-social7.png">
</p>

# Features

- Display messages from multiple streaming platforms at once
- Display avatars of participants of chat
- Displaying the number of viewers both total and for individual platforms
- Displays detailed information about a participant in a live chat in a separate window (including a link to the participant's page/channel)
<p align="center">
  <img src="misc/images/11.png">
</p>

- Sound commands that are activated by the corresponding message from the chat. There are built-in commands, but you can always add your own
- Saving messages and other information on the fly to text files. It may be useful for further processing by other software
- Proxy support
- Cross-platform. However, not all current platforms have builds. Perhaps they will appear in the future, but for now you can make them yourself

# Possibly in future updates
- [x] Twitch support
- [x] YouTube support
- [x] Trovo support
- [x] GoodGame support
- [x] VK Play Live support
- [x] VK Video support
- [x] Kick support
- [ ] TikTok/Douyin support
- [ ] Bigo Live support
- [x] Rumble support
- [x] Odysee support
- [ ] Dlive support
- [x] WASD support
- [x] Discord support
- [x] Telegram support
- [x] DonationAlerts support
- [ ] DonatePay support
- [ ] StreamElements support
- [ ] Streamlabs support
- [ ] Patreon support
- [ ] Boosty support
- [ ] Support for other platforms
- [x] Display avatars of participants of chat
- [x] Displaying the number of viewers both total and for individual platforms
- [x] Displays detailed information about a participant in a live chat in a separate window (including a link to the participant's page/channel)
- [x] Built-in sound commands
- [x] Output data to files in real time. Can be used to integrate with other programs
- [x] Proxy support
- [x] Cross-platform. However, not all current platforms have builds. Perhaps they will appear in the future, but for now you can make them yourself
- [x] Web-based widgets
- [ ] Appearance editor for Web-based widgets
- [x] BetterTTV, FrankerFaceZ, 7TV support on any streaming platform
- [x] Ability to integrate with other applications via WebSocket
- [x] Ability to integrate with other applications via file system
- [x] Windows builds
- [ ] Linux builds
- [ ] macOS builds
- [ ] Android builds
- [ ] iOS builds
- [ ] Many other improvements

# FAQ
- Is AxelChat considered a viewer? - Depending on the version of AxelChat and depending on the streaming platform, this may or may not be the case. If possible, we try not to allow AxelChat to be considered a viewer
- Does AxelChat work with YouTube premieres? - Yes
- Can I download a YouTube chat for an already past stream using the AxelChat? - No, use something else instead. For example, use the browser extension [Save Live Streaming Chats for YouTube™](https://chrome.google.com/webstore/detail/save-live-streaming-chats/bcclhcedlelimlnmcpfeiabljbpahnef)

# How can I help the development of the project
- [Financial support](https://3dproger.github.io/AxelChat/sponsor)
- Recommend to your favorite streamer
- Create new features and fix bugs. For those who know how to C++.  Just make a pull request or fork
- Create and improve widgets using front-end technologies like React. Here is the official widget repository: https://github.com/3dproger/AxelChatWidgets
- Create software that can be integrated into a AxelChat using a websocket or using a file system
- You can report a bug or suggest a new feature in [Issues](https://github.com/3dproger/AxelChat/issues)
- Help with translations into other languages using the [Qt Linguist](https://doc.qt.io/qt-5/qtlinguist-index.html). I can send you translation blank files, just write to me about it

# Known Issues
- YouTube: messages that YouTube may find inappropriate are not displayed
- YouTube: works only with open access broadcasts or available via the link. Private broadcasts are not supported
- YouTube: Doesn't work for streams with age and possibly other restrictions
- Telegram: avatars of users who have disabled the display of avatars in the privacy settings are not displayed

# Troubleshooting
### Doesn't connect (General)
- Check the correctness of the specified stream links/channel name
- Check your internet connection
- Check proxy settings if using
- Check your firewall settings

### Doesn't connect (YouTube)
- Check that the broadcast is not over
- Check that the broadcast is publicly available or accessed via a link. Private broadcasts are not supported

### Twitch: Avatars not showing
- Check that the link to the channel is correct
- Check that the OAuth-token is correct. Update the OAuth-token, if necessary, by pressing the corresponding button in the AxelChat
- Check in the settings of your Twitch account if the permission to use the twitch account has not been revoked. Update the OAuth-token, if necessary, by pressing the corresponding button in the AxelChat

### Control panel web page does not open correctly
- Make sure you're logged into your streaming site in your default browser
- Make sure that you have specified a stream in AxelChat that belongs to your streaming platform account in the default browser, and not someone else's stream

# Build AxelChat from source for Windows 7 and newer, x64
### Installing Qt
1. Download [Open Source Qt online installer](https://www.qt.io/download-qt-installer) or you can also find the offline installers. Use VPN if necessary
1. Run Qt installer and login/sign-up if required
1. While installing select components:
    * Qt / Qt 5.15.x / MinGW x.x.x 64-bit
    * Developer and Designer Tools / Qt Creator x.x.x CDB Debugger Support
    * Developer and Designer Tools / Debugging Tools for Windows
    * Developer and Designer Tools / OpenSSL 1.1.1x Toolkit / OpenSSL 32-bit binaries
    * Developer and Designer Tools / OpenSSL 1.1.1x Toolkit / OpenSSL 64-bit binaries

### Build source for Windows x64
1. Clone repo or download source
1. Create file `src/secrets.h` with contents:
```
#pragma once
#define CRYPTO_KEY_32_BYTES "<think up and insert a 32-character encryption code>"
#define TWITCH_CLIENT_ID "<Paste Twitch client ID here>"
#define TWITCH_SECRET "<Paste Twitch secret here>"
#define TROVO_CLIENT_ID "<Paste Trovo client ID here>"
#define VK_APP_ID "<Paste VK application ID here>"
#define VK_SECURE_KEY "<Paste VK secret here>"
#define WASD_API_TOKEN "<Paste WASD API key here>"
#define DONATIONALERTS_CLIENT_ID "<Paste DonationAlerts client id here>"
#define DONATIONALERTS_API_KEY "<Paste DonationAlerts API key here>"
```
3. Open file `<AxelChat_project_folder>/src/AxelChat.pro` with Qt Creator
1. In Qt Creator select `Desktop Qt x.x.x MinGW 64-bit` kit
1. Select `Release` building mode
1. Click button `Build Project "AxelChat"`. If successful build, folder `release_win64` will appear in the project folder with AxelChat.exe and other files
1. Unpack files from `<AxelChat_project_folder>/app_files/win64.zip` to `<AxelChat_project_folder>/release_win64`
1. To support web widgets, build the project https://github.com/3dproger/AxelChat-react and copy the build result to `<AxelChat_project_folder>/release_win64/widgets`
1. Run `<AxelChat_project_folder>/release_win64/AxelChat.exe` or click `Run` in Qt Creator
1. For subsequent builds, just close `AxelChat.exe` application and click `Run` in Qt Creator

Note: to release an AxelChat for use by other users, I recommend to check if all the required DLLs (\*.so for Linux) are in the folder. [ProcDepMonitor](https://github.com/3dproger/ProcDepMonitor) or [DependencyWalker](https://www.dependencywalker.com/) or analogs can helps you
