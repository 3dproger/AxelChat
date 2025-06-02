# AxelChat API

The AxelChat API provides the ability to integrate third-party software with AxelChat, for example to read messages and viewer information. There are currently two ways to read data from AxelChat: `WebSocket` and `Output to files`

## WebSocket

AxelChat can work as a WebSocket server. At the moment, this is the most preferred method of integration. For example, AxelChat web widgets use this method of communication. The main advantage of this method is instant receipt of messages and various events by your software.

To connect to AxelChat using WebSocket, follow these steps:

* Launch AxelChat
* Open the section `Settings->Developers`. If the WebSocket server was successfully launched, the message "State: Ok" should be displayed
* This section will display the URL for connecting to AxelChat via WebSocket, usually `ws://127.0.0.1:8356`. Connect your WebSocket client using this link. See the documentation for your programming language or use the appropriate libraries
* If the connection is successful, a new client will be displayed in the `Developers` section
* Immediately after connecting, your software should send a message in this format to provide AxelChat with information about your client:
```
{
    "data": {
        "client": {
            "name": "<Name of your client>",
            "version": "<Your client version>"
            "type": "MAIN_WEBSOCKETCLIENT",
        },
        "info": {
            "name": "<Additional client name>",
            "type": "GENERIC"
        }
    },
    "type": "HELLO"
}
```
If you do not send this message within a few seconds, or send another message, the connection will be automatically terminated.

* Next, AxelChat will send a message in the format:
```
{
    "data": {
        "app_name": "AxelChat",
        "app_version": "0.37.3",
        "client_uuid": "{xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx}",
        "services": [...]
	},
	"type": "HELLO"
}
```
The `services` field will display the client ID, which is assigned to the client by AxelChat.

The `services` field stores information about the services currently supported by AxelChat.

* Next, AxelChat will send messages:

### `STATES_CHANGED` - Information about the status of services:

```
{
    "data": {
        "services": [
            {
                "connection_state": "connecting",
                "enabled": true,
                "type_id": "youtube",
                "viewers": 11
            },
            {
                "connection_state": "not_connected",
                "enabled": false,
                "type_id": "twitch",
                "viewers": 13
            }
        ],
        "viewers": 24
    },
    "type": "STATES_CHANGED"
}
```

### `NEW_MESSAGES_RECEIVED`, `MESSAGES_CHANGED` - New messages received or changed existing messages:

```
{
    "data": {
        "messages": [
            {
                "author": {
                    "avatar": "<avatar_url>",
                    "color": "",
                    "customBackgroundColor": "",
                    "id": "<user_id>",
                    "leftBadges": [],
                    "name": "Alex",
                    "pageUrl": "<user_page_url>",
                    "rightBadges": [],
                    "serviceId": "youtube"
                },
                "contents": [
                    {
                        "data": {
                            "text": "Hello, I'm Axel"
                        },
                        "type": "text"
                    }
                ],
                "customAuthorAvatarUrl": "",
                "customAuthorName": "",
				"deleted": false,
                "destination": [],
                "forcedColors": {},
                "id": "<message_id>",
                "publishedAt": "2024-08-04T16:18:02.954",
                "receivedAt": "2024-08-04T16:19:18.548",
                "extra": {}
            },
            {
                "author": {
                    "avatar": "<avatar_url>",
                    "color": "",
                    "customBackgroundColor": "",
                    "id": "<user_id>",
                    "leftBadges": [],
                    "name": "Inn",
                    "pageUrl": "<user_page_url>",
                    "rightBadges": [],
                    "serviceId": "youtube"
                },
                "contents": [
                    {
                        "data": {
                            "text": "See this image "
                        },
                        "type": "text"
                    },
                    {
                        "data": {
                            "url": "https://fonts.gstatic.com/s/e/notoemoji/15.1/1f44c_1f3fc/72.png"
                        },
                        "type": "image"
                    }
                ],
                "customAuthorAvatarUrl": "",
                "customAuthorName": "",
				"deleted": false,
                "destination": [],
                "forcedColors": {},
                "id": "<message_id>",
                "publishedAt": "2024-08-04T16:18:04.069",
                "receivedAt": "2024-08-04T16:19:18.548",
                "extra": {}
            }
        ]
    },
    "type": "NEW_MESSAGES_RECEIVED"
}
```

### `CLEAR_MESSAGES` - Cleared all messages

```
{
    "data": null,
    "type": "CLEAR_MESSAGES"
}
```

New types of messages may appear in future updates of AxelChat. Also, existing message types may be changed, usually by adding new data to the message object.

## Output to files

This method is considered obsolete and may no longer be supported in the future.

To connect to AxelChat using file reading, follow these steps:

* Launch AxelChat
* In the section `Settings->Output to files` enable output to ini-files
* The specified folder will store files that contain various information from AxelChat. These files periodically update their contents and your software must periodically read and parse these files to receive up-to-date information from AxelChat
