#ifndef CHARACTERISTICSMODEL_H
#define CHARACTERISTICSMODEL_H

#include <QBluetoothUuid>
#include <QAbstractListModel>
#include <QPointer>

class QLowEnergyService;
class QLowEnergyCharacteristic;

class CharacteriticsModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(bool running READ isRunning NOTIFY runningChanged)
    Q_PROPERTY(QString errorString READ errorString NOTIFY errorOccurred)

public:
    explicit CharacteriticsModel(QObject *parent = nullptr);

    bool isRunning() const;
    QString errorString() const;

    Q_INVOKABLE void update(QObject *service);

    Q_INVOKABLE void read(const QString &characteristicUuid);
    Q_INVOKABLE void write(const QString &characteristicUuid,
                           const QByteArray &hexValue);

    Q_INVOKABLE void enableNotification(const QString &characteristicUuid,
                                        bool enable);

    Q_INVOKABLE QObject *service() const;

signals:
    void runningChanged(bool running);
    void errorOccurred();

private:
    void setRunning(bool running);

    void updateCharacteristics();

    int rowCount(const QModelIndex &parent) const final;
    QVariant data(const QModelIndex &index, int role) const final;
    QHash<int, QByteArray> roleNames() const final;

    bool m_running = false;
    QPointer<QLowEnergyService> m_service;
    QVector<QBluetoothUuid> m_characteristicUuids;
};

#endif // CHARACTERISTICSMODEL_H
