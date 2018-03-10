#ifndef DEVICESMODEL_H
#define DEVICESMODEL_H

#include <QBluetoothDeviceInfo>
#include <QAbstractListModel>

class QBluetoothDeviceDiscoveryAgent;

class DevicesModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(bool running READ isRunning NOTIFY runningChanged)
    Q_PROPERTY(QString errorString READ errorString NOTIFY errorOccurred)

public:
    explicit DevicesModel(QObject *parent = nullptr);

    bool isRunning() const;
    QString errorString() const;

    Q_INVOKABLE void update();

signals:
    void runningChanged(bool running);
    void errorOccurred();

private:
    void setRunning(bool running);

    int rowCount(const QModelIndex &parent) const final;
    QVariant data(const QModelIndex &index, int role) const final;
    QHash<int, QByteArray> roleNames() const final;

    QBluetoothDeviceDiscoveryAgent *m_discoveryAgent = nullptr;
    bool m_running = false;
    QVector<QBluetoothDeviceInfo> m_devices;
};

#endif // DEVICESMODEL_H
