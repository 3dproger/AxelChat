#pragma once

#include "models/message.h"
#include "models/author.h"
#include <QAbstractListModel>
#include <unordered_map>

class MessagesModel : public QAbstractListModel
{
    Q_OBJECT
public:
    MessagesModel(QObject *parent = 0) : QAbstractListModel(parent) {}

    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    bool removeRows(int position, int rows, const QModelIndex &parent = QModelIndex()) override;

    void append(Message&& message);
    bool contains(const QString& id);
    void clear();

    int getRow(const std::shared_ptr<QVariant>& data);
    const Author* getAuthor(const QString& authorId) const { return _authorsById.value(authorId, nullptr); }
    Author* getAuthor(const QString& authorId) { return _authorsById.value(authorId, nullptr); }
    void insertAuthor(const Author& author);
    void setAuthorValues(const AxelChat::ServiceType serviceType, const QString& authorId, const QMap<Author::Role, QVariant>& values);
    QList<Message> getLastMessages(int count) const;

private:
    QVariant dataByRole(const Message& message, int role) const;
    QModelIndex createIndexByPtr(const std::shared_ptr<QVariant>& data) const;

    QList<std::shared_ptr<QVariant>> _data;
    QHash<QString, std::shared_ptr<QVariant>> _dataById;
    QHash<uint64_t, std::shared_ptr<QVariant>> dataByPosition;
    std::unordered_map<std::shared_ptr<QVariant>, uint64_t> positionByData;

    QHash<QString, Author*> _authorsById;

    uint64_t lastPosition = 0;
    uint64_t removedRows = 0;
};

