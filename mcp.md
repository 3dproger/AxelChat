# MCP (for AI assistants)

Since 0.50.0 AxelChat has a built-in [MCP](https://modelcontextprotocol.io) (Model Context Protocol) server. It allows AI assistants (Claude, Cursor, Cline and others) to read the chat, check the status of platforms and help with troubleshooting. For instance, an AI agent can provide a brief summary of the chat conversation from the last few minutes, gauge the audience's sentiment, read out important messages, perform diagnostics on AxelChat, assist with troubleshooting connection issues, and much more. The potential use cases are vast and limited only by your imagination. For example, you can ask the assistant:
* "What has the chat been talking about for the last 10 minutes?"
* "What questions did viewers ask me?"
* "Why isn't Twitch connecting?"
* "Wait until someone writes `Pay attention to me!` in the chat and tell me"

Address: `http://127.0.0.1:8356/mcp` (transport: Streamable HTTP). AxelChat must be running. The current version of the MCP protocol is shown in `Settings` -> `AI assistants (MCP)`.

Other ways to integrate with AxelChat (HTTP requests, WebSocket) are described in the [AxelChat API documentation](docs.md).

## Quick setup via AI agent

The easiest way to connect is to let the AI agent set itself up:
1. Launch AxelChat
2. Open `Settings` -> `AI assistants (MCP)` and click `Copy prompt`
3. Paste the copied prompt into your AI agent (Claude Code, Cursor, Cline, Codex, etc.) and follow its instructions

The prompt already contains the address and the path to AxelChat on your computer and is in the language of the AxelChat interface. The agent determines which client it is running in, adds the `axelchat` MCP server to its config (showing you the change first) and checks the connection. If the agent fails, connect manually as described in [Connecting](#connecting).

## Settings

The MCP server is configured in `Settings` -> `AI assistants (MCP)`:
* `Enable MCP server` - enabled by default
* `Read only` - enabled by default. AI assistants can only read data, tools that change AxelChat (`send_message`, `edit_message`, `delete_messages`, `clear_chat`, `set_service_enabled`) can't be called. The AI assistant is informed about this mode, so if you ask it to do something that is not allowed, it will tell you which setting to disable
* `Allow access from other devices` - disabled by default. Allows connecting to MCP from other devices on the local network at `http://<IP address of the computer with AxelChat>:8356/mcp`

## Connecting

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

Clients supporting only stdio (e.g. Claude Desktop) launch AxelChat with the `--mcp-stdio` argument. In this mode AxelChat doesn't open windows, it only forwards messages between the client and the already running AxelChat, so AxelChat must be running. Add to the MCP config of the client:
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

## Tools

Chat and state:

| Tool | Description |
|---|---|
| `get_state` | Status of platforms, viewers |
| `get_messages` | Last messages. Parameters: `count` (default 30, max 500), `service` ([platform ID](docs.md#list-of-known-axelchat-platform-ids)), `author`, `afterId` |
| `search_messages` | Search messages by text. Parameters: `query`, `count`, `service`, `afterId` |
| `wait_for_events` | Wait for new messages and changes of platform state, see [Reacting to events](#reacting-to-events). Parameters: `types`, `query`, `author`, `service`, `afterId`, `count`, `timeoutSec` (default 20, max 25) |
| `get_selected_messages` | Messages selected by the streamer |
| `send_message` | Add a new message to AxelChat, similar to [`POST api/v1/receive-events`](docs.md#http-requests) (the message is shown only in AxelChat and its widgets, it is not sent to platforms). Parameters: `text`, `authorName`, `serviceId` or `events` (an event or an array of events in the `receive-events` format, for full control: author badges, colors). If the platform is not specified (`serviceId`, or `author.serviceId` in events), `mcp` is used. It doesn't edit or delete messages: an event with the ID of an existing message or with `markedAsDeleted` returns an error. Returns the IDs of added and ignored messages |
| `edit_message` | Replace the content of an existing message (including messages from platforms) in AxelChat and its widgets, the message is marked as edited (it is not edited on the platform). Parameters: `id`, `text` or `contents` (array in the `contents` format of `receive-events`: text, images, links). Author, time and ID stay the same. Returns the edited message in the [message format](#message-format) |
| `delete_messages` | Delete messages in AxelChat and its widgets (they are not deleted on platforms). Parameters: `ids` (array of message IDs) and/or `userId` (delete all messages of the user, `authorId` of a message). Returns `deletedMessages` (IDs of deleted messages), `alreadyDeletedMessages` (IDs from `ids` that were already deleted), `notFoundMessages` (IDs from `ids` that were not found) and `notFoundUser` (if the user with `userId` was not found) |
| `clear_chat` | Clear the chat in AxelChat and its widgets (doesn't affect platforms) |
| `set_service_enabled` | Enable or disable a platform. Parameters: `instanceId`, `enabled` |

Troubleshooting:

| Tool | Description |
|---|---|
| `get_app_info` | AxelChat version, OS, internet state, number of connected platforms |
| `diagnose_service` | Why a platform doesn't connect: detailed state, notifications shown in the settings, probable reasons and recent log records. Parameters: `instanceId` |
| `list_log_sessions` | List of app launches with logs |
| `read_logs` | Read logs. Parameters: `count`, `minLevel` (`debug`, `info`, `warning`, `critical`, `fatal`), `contains`, `session` (`current`, `previous` or a name from `list_log_sessions`) |

`send_message`, `edit_message`, `delete_messages`, `clear_chat` and `set_service_enabled` change the state of AxelChat, so they are available only if `Read only` is disabled in the settings. They are marked as non-read-only so that the AI client may ask you for confirmation; `delete_messages` and `clear_chat` are additionally marked as destructive. Each action is a separate tool, so in the AI client you can allow, for example, adding messages without confirmation but keep confirmation for deleting. The full up-to-date list of tools with parameters is returned by the `tools/list` method.

`afterId` - ID of a message (e.g. the last message the assistant has seen). If set, only newer messages are returned, starting from the oldest of them (at most `count`), so if exactly `count` messages are returned, there may be more: request again with the ID of the last returned message. Without `afterId` the last `count` messages are returned. If the message with this ID is no longer in AxelChat (e.g. the chat was cleared), the last `count` messages are returned.

## Reacting to events

The assistant can react to events in AxelChat, for example: "Wait until someone writes `Pay attention to me!` in the chat and tell me" or "Tell me if Twitch disconnects". `wait_for_events` uses long polling: AxelChat holds the request until a matching event occurs or `timeoutSec` expires, without blocking the app.

Event types (parameter `types`, an array; by default all types):
* `message` - a new chat message. Filters: `query` (text), `author`, `service` - case-insensitive substrings, except `service` (platform ID). Service messages of AxelChat are ignored. Donations come as messages from donation platforms, so use `service` to wait for them, e.g. `"service": "donationalerts"`
* `service_state` - a platform changed its connection state or was enabled/disabled. Filter: `service` (platform ID or instance ID from `get_state`). Changes of viewers count and settings are not reported

Result:
```JSON
{
    "events": [
        {
            "type": "message",
            "message": {
                "id": "<message_id>",
                "service": "twitch",
                "time": "2026-11-01T07:18:25",
                "text": "Here is the DATA",
                "authorId": "<user_id>",
                "authorName": "<username>"
            }
        },
        {
            "type": "service_state",
            "time": "2026-11-01T07:40:14",
            "instanceId": "kick",
            "service": "kick",
            "connectionState": "connecting",
            "previousConnectionState": "not_connected",
            "enabled": true,
            "previousEnabled": false
        }
    ],
    "lastId": "<message_id>",
    "hasMore": false,
    "timedOut": false
}
```
* `events` - events, oldest first. Empty if the timeout expired. Messages are in the [message format](#message-format)
* `connectionState`, `previousConnectionState` - `not_connected`, `connecting`, `waiting_for_live` or `connected`
* `lastId` - pass it as `afterId` to the next call. Usually it is the ID of the newest message in AxelChat (not necessarily a matching one); if `hasMore` is `true`, it is the ID of the last returned message
* `hasMore` - `true` if more than `count` matching messages are available and only the oldest `count` of them are returned. The next call with `afterId` = `lastId` returns the rest immediately
* `timedOut` - `true` if no matching events occurred in time

The result is returned on the first matching event (several messages that arrived together are returned together). To keep watching, the assistant calls `wait_for_events` in a loop with `afterId` = `lastId` of the previous call, so no messages are missed between calls, even if many messages arrive at once. If there are already matching messages after `afterId`, the result is returned immediately. Without `afterId` only messages that arrive after the call are returned. Platform state events are not stored: they are returned only if they occur during the call, use `get_state` to get the current state.

`timeoutSec` is limited to 25 seconds, because MCP clients and the stdio mode usually stop waiting for a reply after 30 seconds. `wait_for_events` can't be called in a JSON-RPC batch, call it in a separate request.

Example (waits up to 20 seconds for a message containing `Pay attention to me!`):
```
curl -X POST http://127.0.0.1:8356/mcp -H "Content-Type: application/json" -d "{\"jsonrpc\":\"2.0\",\"id\":1,\"method\":\"tools/call\",\"params\":{\"name\":\"wait_for_events\",\"arguments\":{\"types\":[\"message\"],\"query\":\"DATA\",\"timeoutSec\":20}}}"
```

Moderation example: "Watch the chat and delete messages with links". The assistant calls `wait_for_events` in a loop and, for each unwanted message, calls `delete_messages` with its `id` (or with `userId` = `authorId` to delete all messages of the author):
```
curl -X POST http://127.0.0.1:8356/mcp -H "Content-Type: application/json" -d "{\"jsonrpc\":\"2.0\",\"id\":2,\"method\":\"tools/call\",\"params\":{\"name\":\"delete_messages\",\"arguments\":{\"ids\":[\"<message_id>\"]}}}"
```
Messages are deleted only in AxelChat and its widgets (e.g. in OBS), they stay on the platform and the author is not banned. Requires `Read only` to be disabled.

## Resources

* `axelchat://state` - status of platforms and viewers
* `axelchat://messages/recent` - last 30 messages
* `axelchat://messages/selected` - selected messages

## Message format

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

## Testing without an AI client

MCP is JSON-RPC 2.0 over HTTP POST, so you can test it with Postman or curl:
```
curl -X POST http://127.0.0.1:8356/mcp -H "Content-Type: application/json" -d "{\"jsonrpc\":\"2.0\",\"id\":1,\"method\":\"tools/call\",\"params\":{\"name\":\"get_messages\",\"arguments\":{\"count\":5}}}"
```
Sending a message (requires `Read only` to be disabled):
```
curl -X POST http://127.0.0.1:8356/mcp -H "Content-Type: application/json" -d "{\"jsonrpc\":\"2.0\",\"id\":2,\"method\":\"tools/call\",\"params\":{\"name\":\"send_message\",\"arguments\":{\"text\":\"Hello from AI\",\"authorName\":\"Assistant\"}}}"
```

For interactive testing use [MCP Inspector](https://github.com/modelcontextprotocol/inspector): `npx @modelcontextprotocol/inspector`

## Security and privacy

* By default MCP is available only from the computer where AxelChat is running. If you enable `Allow access from other devices`, anyone on your local network will be able to read the chat and logs of AxelChat, and also change it if `Read only` is disabled
* Requests from web pages of other sites are rejected
* Tokens are not returned. For donation platforms whose connection parameter is a secret token, only the fact that the parameter is set is returned
* Keep in mind that chat messages and logs are sent to the AI provider you use

## Limitations

* Server-to-client notifications (SSE) are not supported: the assistant receives new messages only when it requests them. To react to events, use `wait_for_events`, but the assistant (or its client) still has to call it again after each result
* Only chat messages and platform state changes can be waited for. Donations, subscriptions, raids and other platform events are not separate event types yet, they come as chat messages
* The set of tools may change in future versions
