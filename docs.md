# AxelChat API

The AxelChat API provides the ability to integrate third-party software with AxelChat, for example to read messages and viewer information. There are currently four ways to read data from AxelChat: `HTTP requests`, `WebSocket`, `MCP` (for AI assistants) and `Output to files`

## HTTP requests

AxelChat can work as a HTTP server. Using HTTP requests, you can receive various data or control the work of AxelChat. For ease of API testing, use Postman or similar applications. Here is a list of HTTP requests supported by the latest version of AxelChat:
* GET `http://127.0.0.1:8356/api/v1/set-visibility?main=0` - hide main window AxelChat
* GET `http://127.0.0.1:8356/api/v1/set-visibility?main=1` - show main window AxelChat
* GET `http://127.0.0.1:8356/api/v1/app` - get information about AxelChat version
* GET `http://127.0.0.1:8356/api/v1/messages?count=100` - get list of last 100 messages
* GET `http://127.0.0.1:8356/api/v1/selected-messages` - get list of selected messages
* GET `http://127.0.0.1:8356/api/v1/state` - get information about the status of connected platforms, including information about the number of viewers and more
* GET `http://127.0.0.1:8356/api/v1/chat/clear` - (since 0.40.5) clear chat
* POST `http://127.0.0.1:8356/api/v1/receive-events` - (since 0.44.0) allows adding new messages to AxelChat and modifying or deleting (since 0.47.6) existing ones. This endpoint can be used both for sending test events and for integration with a third-party program that receives data from a platform that is not natively supported in AxelChat, or for displaying any other messages. The request body must contain an array of objects. Each object is an event, such as a message event. Please keep some points in mind:
	- Support for deleting messages was introduced only in version 0.47.6.
  	- It is not necessary to specify all fields in the object
 	- If you don't specify an event ID or message author ID, AxelChat will generate them automatically
  	- It is recommended to additionally specify the platform ID at the beginning of the author ID to avoid identifier conflicts. For message IDs, it is also recommended to add the author ID at the beginning
 	- If you send two messages with the same ID, the second message will be ignored. However, if the `edited` field or `markedAsDeleted` field (since 0.47.6) in the second message is `true`, the first message will be overwritten or deleted by the second message, and a corresponding note will be added indicating that the message has been edited
  - The `serviceId` field contains the text ID of the platform from which this event was received. If the event was received from a platform AxelChat doesn't know anything about, you can create your own ID. In this case, it is recommended to also specify the URL to the platform's icon in the `serviceBadge` field. A list of known AxelChat platform IDs can be found below in this documentation.
  - If this request is successfully processed, AxelChat will return information about how the event was handled (the IDs of the messages that were added, updated, or deleted). A sample response is below.
  - Support for the `reply` field was introduced in version 0.47.6. The fields of this object are optional, although it is recommended to specify them if possible.
  - Starting with version 0.48.1, you can send a JSON object representing a single message. Previously, it was only possible to send an array of JSON objects, even if you needed to transmit just one message.

An example of two new message events:
```JSON
[
    {
        "contents": [
            {
                "data": {
                    "text": "Simple message"
                },
                "type": "text"
            }
        ]
    },
    {
        "author": {
            "id": "twitch_15VIhcy2f6qvy6eKaI79d",
            "name": "Goose Doe",
            "serviceId": "twitch",
            "visible": {
                "private": true,
                "public": true
            },
            "color": "#FF3FEF",
            "customBackgroundColor": "#99D9FF",
            "avatar": "https://img.icons8.com/?size=100&id=CqciCrEqfguR&format=png&color=000000",
            "page": "https://en.wikipedia.org/wiki/Goose",
            "serviceBadge": "https://img.icons8.com/?size=100&id=3XZiNbyjUeNL&format=png&color=000000",
            "leftBadges": [
                "https://img.icons8.com/?size=100&id=0WWwdtiSb6xB&format=png&color=000000",
                "https://img.icons8.com/?size=100&id=81210&format=png&color=000000"
            ],
            "leftTags": [
                {
                    "text": "GOOSE",
                    "backgroundColor": "#990026",
                    "textColor": "#FFFFFF"
                }
            ],
            "rightBadges": [
                "https://img.icons8.com/?size=100&id=101716&format=png&color=000000",
                "https://img.icons8.com/?size=100&id=120346&format=png&color=000000"
            ],
            "rightTags": [
                {
                    "text": "STREAMER",
                    "backgroundColor": "#9900E4",
                    "textColor": "#FFFFFF"
                }
            ]
        },
        "id": "twitch_15VIhcy2f6qvy6eKaI79d_15VIhcy2f6qvy6eKaI79_9BPXd1Q75B65zHgnQppd7Mjg7m3G7Zv",
        "eventType": "Message",
        "edited": false,
        "deletedOnPlatform": false,
        "markedAsDeleted": false,
        "customAuthorAvatarUrl": "",
        "customAuthorName": "",
        "mentionedYouAs": "",
        "publishedAt": "2025-12-27T12:13:47.266",
        "receivedAt": "2025-12-27T12:43:48.145",
        "visible": {
            "private": true,
            "public": true
        },
        "reply": {
            "name": "Bob the Beaver",
            "text": "Hi, I finished the dam!",
            "userId": "twitch_CnjakAS7jHAJch23",
            "messageId": "twitch_CnjakAS7jHAJch23_VAM87NAI1ni92nmVA6289Bajh"
        },
        "bodyStyle": {
            "backgroundColor": "#0000FF",
            "borderColor": "#00FF00",
            "sideLineColor": "#FF0000"
        },
        "contents": [
            {
                "data": {
                    "text": "Hi, Bob the Beaver! I'm Goose Doe\n"
                },
                "style": {
                    "font-weight": "bold"
                },
                "type": "text"
            },
            {
                "data": {
                    "text": "This is my friend, Duckinson Duck: "
                },
                "style": {
                    "font-style": "italic"
                },
                "type": "text"
            },
            {
                "data": {
                    "url": "https://img.icons8.com/?size=100&id=BO1UW1ckftwU&format=png&color=000000"
                },
                "type": "image"
            },
            {
                "data": {
                    "text": "\nI found his photo "
                },
                "type": "text"
            },
            {
                "data": {
                    "text": "here",
                    "url": "https://icons8.com/icons/set/duck"
                },
                "style": {
                    "font-style": "italic"
                },
                "type": "hyperlink"
            },
            {
                "data": {
                    "text": "\nAnd this is arbitrary HTML content:\n"
                },
                "type": "text"
            },
            {
                "data": {
                    "html": "This is another friend of mine, Platy Platypus:<br><img src=\"https://img.icons8.com/?size=100&id=Ssxi5U7QWAK4&format=png&color=000000\">"
                },
                "type": "html"
            }
        ]
    }
]
```

Response example:
```JSON
{
    "addedMessages": [
        "twitch_15VIhcy2f6qvy6eKaI79_9BPXd1Q75B65zHgnQppd7Mjg7m3G7Zv"
    ],
    "ignoredMessages": [],
    "removedMessages": [],
    "updatedMessages": []
}
```
Note: The `ignoredMessages` field was added in version 0.48.1.

An example of deleting one of the messages from the previous example (works since 0.47.6):
```JSON
[
    {
        "id": "twitch_15VIhcy2f6qvy6eKaI79_9BPXd1Q75B65zHgnQppd7Mjg7m3G7Zv",
        "markedAsDeleted": true
    }
]
```
  
## WebSocket

AxelChat can work as a WebSocket server. AxelChat web widgets use exactly this method of communication. The main advantage of this method is instant receipt of messages and various events by your software.

To connect to AxelChat using WebSocket, follow these steps:

* Launch AxelChat
* Open the section `Settings->Developers`. If the WebSocket server was successfully launched, the message "State: Ok" should be displayed
* This section will display the URL for connecting to AxelChat via WebSocket, usually `ws://127.0.0.1:8356`. Connect your WebSocket client using this link. See the documentation for your programming language or use the appropriate libraries
* If the connection is successful, a new client will be displayed in the `Developers` section
* (DEPRECATED since 0.47.6) Immediately after connecting, your software should send a message in this format to provide AxelChat with information about your client:
```JSON
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
(DEPRECATED since 0.47.6) If you do not send this message within a few seconds, or send another message, the connection will be automatically terminated.

* Next, AxelChat will send a message in the format (since 0.47.6):
```JSON
{
	"data": {
		"app": {
			"locale": "en-US",
			"name": "AxelChat",
			"version": "0.47.6"
		},
		"client": {
			"address": "127.0.0.1",
			"executablePath": "<path-to-executable>",
			"executableVersion": "<executable-version>",
			"local": true,
			"pid": 12345,
			"port": 54321
		},
		"services": [...]
	},
	"type": "HELLO"
}
```

* Before 0.47.6 this message looks like this:
```JSON
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

Next, AxelChat will send other messages (see below).

### `SERVER_ALIVE` - (since 0.47.6) The server reports that it is active
If the server fails to send this message for an extended period, it indicates that the server has shut down, frozen, or that the connection to the server has been severed. Message example:
```JSON
{
    "data": null,
    "type": "SERVER_ALIVE"
}
```

### `STATES_CHANGED` - Information about the status of services:

```JSON
{
    "data": {
        "services": [
            {
                "connection_state": "connecting",
                "enabled": true,
                "type_id": "youtube",
				"icon": "/resources?path=qrc:/resources/images/youtube-icon.svg",
                "viewers": 11,
				"viewersCounterEnabled": true,
				"followers": -1
            },
            {
                "connection_state": "not_connected",
                "enabled": false,
                "type_id": "twitch",
				"icon": "/resources?path=qrc:/resources/images/twitch-icon.svg",
                "viewers": 13,
				"viewersCounterEnabled": true,
				"followers": 123
            }
        ],
        "viewers": 24
    },
    "type": "STATES_CHANGED"
}
```

### `NEW_MESSAGES_RECEIVED`, `MESSAGES_CHANGED` - New messages received or changed existing messages:

```JSON
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
                "bodyStyle": {},
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
                "bodyStyle": {},
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

```JSON
{
    "data": null,
    "type": "CLEAR_MESSAGES"
}
```

### Notification of changes

New message types may appear in future updates of AxelChat. Also, existing message types may be changed, usually by adding new data to the message object. Stay tuned for updates

## MCP (for AI assistants)

Since 0.50.0 AxelChat has a built-in [MCP](https://modelcontextprotocol.io) (Model Context Protocol) server. It allows AI assistants (Claude, Cursor, Cline and others) to read the chat, check the status of platforms and help with troubleshooting. For instance, an AI agent can provide a brief summary of the chat conversation from the last few minutes, gauge the audience's sentiment, read out important messages, perform diagnostics on AxelChat, assist with troubleshooting connection issues, and much more. The potential use cases are vast and limited only by your imagination. For example, you can ask the assistant:
* "Monitor the AxelChat chat, provide me with a summary of the audience's sentiment every 5 minutes, and also read out important messages aloud"
* "What has the chat been talking about for the last 10 minutes?"
* "What questions did viewers ask me?"
* "Why isn't Twitch connecting?"

Address: `http://127.0.0.1:8356/mcp` (transport: Streamable HTTP, protocol version `2025-03-26`). AxelChat must be running.

### Settings

The MCP server is configured in `Settings` -> `AI assistants (MCP)`:
* `Enable MCP server` - enabled by default
* `Read only` - enabled by default. AI assistants can only read data, tools that change AxelChat (`clear_chat`, `set_service_enabled`) can't be called. The AI assistant is informed about this mode, so if you ask it to do something that is not allowed, it will tell you which setting to disable
* `Allow access from other devices` - disabled by default. Allows connecting to MCP from other devices on the local network, the addresses are shown on the same settings page

### Connecting

Claude Code:
```
claude mcp add --transport http axelchat http://127.0.0.1:8356/mcp
```

Clients supporting HTTP transport (Cursor, Cline, etc.), add to the MCP config:
```JSON
{
    "mcpServers": {
        "axelchat": {
            "url": "http://127.0.0.1:8356/mcp"
        }
    }
}
```

Clients supporting only stdio (e.g. Claude Desktop) launch AxelChat with the `--mcp-stdio` argument. In this mode AxelChat doesn't open windows, it only forwards messages between the client and the already running AxelChat, so AxelChat must be running. The ready-made config with the path to AxelChat on your computer can be copied in `Settings` -> `AI assistants (MCP)`:
```JSON
{
    "mcpServers": {
        "axelchat": {
            "command": "<path to AxelChat>",
            "args": ["--mcp-stdio"]
        }
    }
}
```
Path to AxelChat:
* Windows: `<path to AxelChat folder>\\AxelChat.exe` (the folder where AxelChat is installed, backslashes in JSON must be doubled)
* macOS usually: `/Applications/AxelChat.app/Contents/MacOS/AxelChat`
* Linux: path to the AppImage file

### Tools

Chat and state:

| Tool | Description |
|---|---|
| `get_state` | Status of platforms, viewers |
| `get_messages` | Last messages. Parameters: `count` (default 30, max 500), `service` (platform ID), `author` |
| `search_messages` | Search messages by text. Parameters: `query`, `count`, `service` |
| `get_selected_messages` | Messages selected by the streamer |
| `clear_chat` | Clear the chat in AxelChat (doesn't affect platforms) |
| `set_service_enabled` | Enable or disable a platform. Parameters: `instanceId`, `enabled` |

Troubleshooting:

| Tool | Description |
|---|---|
| `get_app_info` | AxelChat version, OS, internet state, number of connected platforms |
| `diagnose_service` | Why a platform doesn't connect: detailed state, notifications shown in the settings, probable reasons and recent log records. Parameters: `instanceId` |
| `list_log_sessions` | List of app launches with logs |
| `read_logs` | Read logs. Parameters: `count`, `minLevel` (`debug`, `info`, `warning`, `critical`, `fatal`), `contains`, `session` (`current`, `previous` or a name from `list_log_sessions`) |

`clear_chat` and `set_service_enabled` change the state of AxelChat, so they are available only if `Read only` is disabled in the settings. They are marked as non-read-only so that the AI client may ask you for confirmation. The full up-to-date list of tools with parameters is returned by the `tools/list` method.

### Resources

* `axelchat://state` - status of platforms and viewers
* `axelchat://messages/recent` - last 30 messages
* `axelchat://messages/selected` - selected messages

### Message format

Unlike the HTTP API, messages are returned in a compact form to save AI context: no styles and HTML, images are replaced with their text (emoji, emote code) if known, otherwise with `[image]`:
```JSON
{
    "id": "<message_id>",
    "service": "youtube",
    "time": "2026-11-01T00:11:58",
    "text": "omg it's so funny 😭😭",
    "authorId": "<user_id>",
    "authorName": "<username>"
}
```

### Testing without an AI client

MCP is JSON-RPC 2.0 over HTTP POST, so you can test it with Postman or curl:
```
curl -X POST http://127.0.0.1:8356/mcp -H "Content-Type: application/json" -d "{\"jsonrpc\":\"2.0\",\"id\":1,\"method\":\"tools/call\",\"params\":{\"name\":\"get_messages\",\"arguments\":{\"count\":5}}}"
```
For interactive testing use [MCP Inspector](https://github.com/modelcontextprotocol/inspector): `npx @modelcontextprotocol/inspector`

### Security and privacy

* By default MCP is available only from the computer where AxelChat is running. If you enable `Allow access from other devices`, anyone on your local network will be able to read the chat and logs of AxelChat, and also change it if `Read only` is disabled
* Requests from web pages of other sites are rejected
* Tokens are not returned. For donation platforms whose connection parameter is a secret token, only the fact that the parameter is set is returned
* Keep in mind that chat messages and logs are sent to the AI provider you use

### Limitations

* Server-to-client notifications (SSE) are not supported: the assistant receives new messages only when it requests them
* The set of tools may change in future versions

## Output to files

This method is considered obsolete and may no longer be supported in the future.

To connect to AxelChat using file reading, follow these steps:

* Launch AxelChat
* In the section `Settings->Output to files` enable output to ini-files
* The specified folder will store files that contain various information from AxelChat. These files periodically update their contents and your software must periodically read and parse these files to receive up-to-date information from AxelChat

## List of known AxelChat platform IDs
This list is current as of version 0.44.0
```JSON
["bigolive", "boosty", "chzzk", "discord", "dlive", "donatepayeu", "donatepayru", "donatestream", "donationalerts", "facebook", "goodgame", "kick", "livacha", "loco", "memealerts", "nekolive", "niconico", "nimotv", "nuum", "odysee", "ok", "parti", "picarto", "rumble", "rutube", "software", "soopglobal", "soopkorea", "steam", "streamelements", "streamlabs", "telegram", "tiktok", "trovo", "twitch", "unknown", "vaughnlive", "vimm", "vkvideo", "vkvideolive", "xcom", "youtube", "youtubeshorts"]
```
You can find the most up-to-date list of platforms for your version this way:
1. Enable `Output to a files` (`Settings`->`Output to a files`)
2. Click `Show in Explorer`
3. In the folder that opens, open the folder `services`. This folder will list folders whose names correspond to the IDs of known AxelChat platforms except for IDs `software` and `unknown`
