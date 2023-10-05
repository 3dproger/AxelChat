#pragma once

#include "utils.h"
#include "setting.h"
#include "chatservicestypes.h"
#include "uibridge.h"
#include "tcprequest.h"
#include "tcpreply.h"
#include "tcpserver.h"
#include "models/author.h"
#include "CefWebEngineQt/Manager.h"
#include <QSettings>
#include <QObject>
#include <QQmlEngine>
#include <QCoreApplication>
#include <QJsonObject>
#include <QJsonArray>
#include <set>

class ChatHandler;
class Author;
class Message;

class ChatService : public QObject
{
    Q_OBJECT

public:
    Q_PROPERTY(bool                 enabled                      READ isEnabled     WRITE setEnabled     NOTIFY stateChanged)
    Q_PROPERTY(QUrl                 streamUrl                    READ getStreamUrl                       NOTIFY stateChanged)
    Q_PROPERTY(QUrl                 chatUrl                      READ getChatUrl                         NOTIFY stateChanged)
    Q_PROPERTY(QUrl                 controlPanelUrl              READ getControlPanelUrl                 NOTIFY stateChanged)

    Q_PROPERTY(ConnectionStateType  connectionStateType          READ getConnectionState                 NOTIFY stateChanged)
    Q_PROPERTY(QString              stateDescription             READ getStateDescription                NOTIFY stateChanged)

    Q_PROPERTY(int                  viewersCount                 READ getViewersCount                    NOTIFY stateChanged)

    Q_PROPERTY(bool  enabledThirdPartyEmotes      READ isEnabledThirdPartyEmotes   WRITE setEnabledThirdPartyEmotes     NOTIFY stateChanged)

    enum class ConnectionStateType
    {
        NotConnected = 10,
        Connecting = 20,
        Connected = 30
    };
    Q_ENUM(ConnectionStateType)

    struct State
    {
        bool connected = false;
        QString streamId;
        QUrl streamUrl;
        QUrl chatUrl;
        QUrl controlPanelUrl;
        int viewers = -1;
    };

    static const QString UnknownBadge;

    explicit ChatService(QSettings& settings, const QString& settingsGroupPathParent, AxelChat::ServiceType serviceType_, const bool enabledThirdPartyEmotesDefault, QObject *parent = nullptr);
    virtual ~ChatService(){}

    ChatService (const ChatService&) = delete;
    ChatService (ChatService&&) = delete;
    ChatService& operator= (const ChatService&) = delete;
    ChatService&& operator= (ChatService&&) = delete;

    static QString getServiceTypeId(const AxelChat::ServiceType serviceType);
    static QString getName(const AxelChat::ServiceType serviceType);
    static QUrl getIconUrl(const AxelChat::ServiceType serviceType);

    QUrl getChatUrl() const;
    QUrl getControlPanelUrl() const;
    Q_INVOKABLE QUrl getStreamUrl() const;

    virtual ConnectionStateType getConnectionState() const = 0;
    virtual QString getStateDescription() const  = 0;
    virtual TcpReply processTcpRequest(const TcpRequest& request);
    AxelChat::ServiceType getServiceType() const;

    void reconnect();

    int getViewersCount() const;

    bool isEnabled() const;
    void setEnabled(const bool enabled);

    bool isEnabledThirdPartyEmotes() const;
    void setEnabledThirdPartyEmotes(const bool enabled);

    const Setting<QString>& getLastSavedMessageId() const;
    void setLastSavedMessageId(const QString& messageId);

    QJsonObject getStateJson() const;
    QJsonObject getStaticInfoJson() const;

    const State& getState() const;

    Q_INVOKABLE QString getName() const;
    Q_INVOKABLE QUrl getIconUrl() const;

    Q_INVOKABLE UIBridge* getUiBridge() const { return &const_cast<UIBridge&>(ui); }

#ifdef QT_QUICK_LIB
    static void declareQml()
    {
        qmlRegisterUncreatableType<ChatService> ("AxelChat.ChatService", 1, 0, "ChatService", "Type cannot be created in QML");
    }
#endif

signals:
    void stateChanged();
    void readyRead(const QList<std::shared_ptr<Message>>& messages, const QList<std::shared_ptr<Author>>& authors);
    void authorDataUpdated(const QString& authorId, const QMap<Author::Role, QVariant>& values);

private slots:
    void onUIElementChanged(const std::shared_ptr<UIBridgeElement>& element);

private:
    const AxelChat::ServiceType serviceType;
    const QString settingsGroupPath;

protected:

    virtual void reconnectImpl() = 0;

    QString generateAuthorId(const QString& rawId) const;
    QString generateMessageId(const QString& rawId) const;

    void setConnected(const bool connected);
    bool isConnected() const;

    std::shared_ptr<Author> getServiceAuthor() const;

    const QString& getSettingsGroupPath() const { return settingsGroupPath; }

    State state;
    Setting<bool> enabled;
    Setting<QString> stream;
    Setting<QString> lastSavedMessageId;
    Setting<bool> enabledThirdPartyEmotes;

    UIBridge ui;
};
