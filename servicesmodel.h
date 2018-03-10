#ifndef SERVICESMODEL_H
#define SERVICESMODEL_H

#include <QAbstractListModel>
#include <QPointer>

class QLowEnergyService;
class QLowEnergyController;

class ServicesModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(bool running READ isRunning NOTIFY runningChanged)
    Q_PROPERTY(bool connected READ isConnected NOTIFY connectedChanged)
    Q_PROPERTY(QString errorString READ errorString NOTIFY errorOccurred)

public:
    explicit ServicesModel(QObject *parent = nullptr);

    bool isRunning() const;
    bool isConnected() const;
    QString errorString() const;

    Q_INVOKABLE void update(const QString &deviceAddress);
    Q_INVOKABLE QObject *service(const QString &serviceUuid) const;

signals:
    void runningChanged(bool running);
    void connectedChanged(bool connected);
    void errorOccurred();

private:
    void setRunning(bool running);
    void setConnected(bool connected);

    int rowCount(const QModelIndex &parent) const final;
    QVariant data(const QModelIndex &index, int role) const final;
    QHash<int, QByteArray> roleNames() const final;

    bool m_running = false;
    bool m_connected = false;
    QVector<QLowEnergyService *> m_services;
    QPointer<QLowEnergyController> m_controller;
};

#endif // SERVICESMODEL_H
