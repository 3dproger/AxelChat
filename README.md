# AxelChat <img width="32" height="32" src="misc/images/youtube-icon.svg"> <img width="32" height="32" src="misc/images/twitch-icon.svg"> <img width="32" height="32" src="misc/images/trovo-icon.svg"> <img width="32" height="32" src="misc/images/kick-icon.svg"> <img width="32" height="32" src="misc/images/rumble-icon.svg"> <img width="32" height="32" src="misc/images/dlive-icon.svg"> <img width="32" height="32" src="misc/images/tiktok-icon.svg"> <img width="32" height="32" src="misc/images/odysee-icon.svg"> <img width="32" height="32" src="misc/images/goodgame-icon.svg"> <img width="32" height="32" src="misc/images/vkplaylive-icon.svg"> <img width="32" height="32" src="misc/images/vkvideo-icon.svg"> <img width="32" height="32" src="misc/images/nuum-icon.svg"> <img width="32" height="32" src="misc/images/rutube-icon.svg"> <img width="32" height="32" src="misc/images/vimm-icon.svg"> <img width="32" height="32" src="misc/images/telegram-icon.svg"> <img width="32" height="32" src="misc/images/discord-icon.svg"> <img width="32" height="32" src="misc/images/donationalerts-icon.svg"> <img width="32" height="32" src="misc/images/donatepay-icon.svg">
The AxelChat application collects chat from various streaming platforms and displays it in one window. Has a user-friendly interface and various useful features

[Download](https://github.com/3dproger/axelchat/releases/latest/) |
[Releases](https://github.com/3dproger/AxelChat/releases) |
[Feedback](https://docs.google.com/forms/d/e/1FAIpQLSeWahZ2AiyrPNtoF7LLrs3hO4ylYJmth4PmNvjSGsDxiwuwZg/viewform?usp=sf_link) |
[Issues](https://github.com/3dproger/AxelChat/issues) |
[Sponsor](https://3dproger.github.io/AxelChat/sponsor) |
[Privacy Policy](https://3dproger.github.io/AxelChat/privacy) |
[Telegram](https://t.me/axelchatstreaming)

[<img src="misc/images/button-download.png">](https://github.com/3dproger/AxelChat/releases)
[<img src="misc/images/button-feedback.png">](https://docs.google.com/forms/d/e/1FAIpQLSeWahZ2AiyrPNtoF7LLrs3hO4ylYJmth4PmNvjSGsDxiwuwZg/viewform?usp=sf_link)
[<img src="misc/images/button-support.png">](https://3dproger.github.io/AxelChat/sponsor)

<p align="center">
  <img src="misc/github-social8.png">
</p>

### In OBS
<p align="center">
  <img src="misc/screenshots/13.png">
</p>

### Chat participants
<p align="center">
  <img src="misc/screenshots/11.png">
</p>

# Supported and planned platforms
- [x] Twitch
- [x] YouTube
- [x] Trovo
- [x] GoodGame
- [x] VK Play Live
- [x] VK Video
- [x] Kick
- [x] Rumble
- [x] Odysee
- [x] DLive
- [x] TikTok
- [x] NUUM
- [x] Rutube
- [x] VIMM
- [x] Discord
- [x] Telegram
- [x] DonationAlerts
- [x] DonatePay
- [x] BetterTTV
- [x] FrankerFaceZ
- [x] 7TV
- [ ] Bigo Live
- [ ] Facebook Live
- [ ] X / Twitter
- [ ] Steam
- [ ] VStream
- [ ] shareplay.tv
- [ ] live.space
- [ ] Nimo TV
- [ ] Huya Live
- [ ] Douyin
- [ ] StreamElements
- [ ] Streamlabs
- [ ] Patreon
- [ ] Boosty
- [ ] Chatterino Homies Edition
- [ ] Many others. **You can suggest [here](https://docs.google.com/forms/d/e/1FAIpQLSeWahZ2AiyrPNtoF7LLrs3hO4ylYJmth4PmNvjSGsDxiwuwZg/viewform) or [here](https://github.com/3dproger/AxelChat/issues)**

# Features
- [x] Display messages from multiple streaming platforms at once
- [x] Display avatars of participants of chat
- [x] Displaying the number of viewers both total and for individual platforms
- [x] Displays detailed information about a participant in a live chat in a separate window including a link to the participant's page/channel (see example below)
- [x] Sound commands that are activated by the corresponding message from the chat. There are built-in commands, but you can always add your own
- [x] Saving messages and other information on the fly to text files. It may be useful for further processing by other software
- [x] BetterTTV, FrankerFaceZ, 7TV emotes support on any streaming platform
- [x] Ability to integrate with other applications via WebSocket
- [x] Ability to integrate with other applications via file system
- [x] Windows releases
- [ ] Cross-platform
- [ ] Web-based widgets
- [ ] Appearance editor for Web-based widgets
- [ ] Appearance presets (themes) for widgets
- [ ] Linux releases
- [ ] macOS releases
- [ ] Android releases
- [ ] iOS releases
- [ ] Many other improvements

# FAQ
- Is AxelChat considered a viewer? - Depending on the version of AxelChat and depending on the streaming platform, this may or may not be the case. If possible, we try not to allow AxelChat to be considered a viewer
- Does AxelChat work with YouTube premieres? - Yes
- Can I download a YouTube chat for an already past stream using the AxelChat? - No, use something else instead. For example, use the browser extension [Save Live Streaming Chats for YouTube™](https://chrome.google.com/webstore/detail/save-live-streaming-chats/bcclhcedlelimlnmcpfeiabljbpahnef)

# How can I help the development of the project
- [Financial support](https://3dproger.github.io/AxelChat/sponsor)
- Recommend to your favorite streamer
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
- Try to use VPN
- Check proxy settings if using
- Check your firewall settings

### Twitch: Avatars not showing
- Check that the link to the channel is correct
- Check that the OAuth-token is correct. Update the OAuth-token, if necessary, by pressing the corresponding button in the AxelChat
- Check in the settings of your Twitch account if the permission to use the twitch account has not been revoked. Update the OAuth-token, if necessary, by pressing the corresponding button in the AxelChat

### Control panel web page does not open correctly
- Make sure you're logged into your streaming site in your default browser
- Make sure that you have specified a stream in AxelChat that belongs to your streaming platform account in the default browser, and not someone else's stream
