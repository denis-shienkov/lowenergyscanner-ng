#include "devicesmodel.h"

#include <QBluetoothDeviceDiscoveryAgent>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(BLE_DEVICES_MODEL)

enum {
    DeviceNameRole = Qt::UserRole + 1,
    DeviceAddressRole
};

DevicesModel::DevicesModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_discoveryAgent(new QBluetoothDeviceDiscoveryAgent(this))
{
    connect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::canceled,
            [this]() {
        setRunning(false);
    });

    connect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::finished,
            [this]() {
        setRunning(false);
    });

    connect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered,
            [this](const QBluetoothDeviceInfo &device) {
        if (m_devices.contains(device)) {
            qCWarning(BLE_DEVICES_MODEL) << "Nothing to add, device already is in model:"
                                         << device.name();
            return;
        }

        qCDebug(BLE_DEVICES_MODEL) << "Add device:" << device.name();
        const auto rowsCount = m_devices.count();
        beginInsertRows(QModelIndex(), rowsCount, rowsCount);
        m_devices.append(device);
        endInsertRows();
    });

    connect(m_discoveryAgent, QOverload<QBluetoothDeviceDiscoveryAgent::Error>::of(
                &QBluetoothDeviceDiscoveryAgent::error),
            this, &DevicesModel::errorOccurred);
}

int DevicesModel::discoveryTimeout() const
{
    return m_discoveryAgent->lowEnergyDiscoveryTimeout();
}

void DevicesModel::setDiscoveryTimeout(int discoveryTimeout)
{
    if (m_discoveryAgent->lowEnergyDiscoveryTimeout() == discoveryTimeout)
        return;
    m_discoveryAgent->setLowEnergyDiscoveryTimeout(discoveryTimeout);
    qCDebug(BLE_DEVICES_MODEL) << "Set discovery timeout:" << discoveryTimeout;
    emit discoveryTimeoutChanged(discoveryTimeout);
}

bool DevicesModel::isRunning() const
{
    return m_running;
}

void DevicesModel::setRunning(bool running)
{
    if (m_running == running)
        return;
    m_running = running;
    qCDebug(BLE_DEVICES_MODEL) << "Set running:" << m_running;
    emit runningChanged(m_running);
}

QString DevicesModel::errorString() const
{
    return m_discoveryAgent->errorString();
}

void DevicesModel::update()
{
    if (m_running)
        return;
    qCDebug(BLE_DEVICES_MODEL) << "Start devices discovery";
    setRunning(true);
    m_discoveryAgent->start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);
}

int DevicesModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_devices.count();
}

QVariant DevicesModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0)
        return QVariant();
    if (index.row() >= m_devices.count())
        return QVariant();

    const auto row = index.row();
    const auto &device = m_devices.at(row);
    switch (role) {
    case DeviceNameRole:
        return device.name();
    case DeviceAddressRole:
        return device.address().toString();
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> DevicesModel::roleNames() const
{
    return {
        { DeviceNameRole, "name" },
        { DeviceAddressRole, "address" }
    };
}
