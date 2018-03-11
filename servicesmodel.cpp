#include "servicesmodel.h"

#include <QLowEnergyService>
#include <QLowEnergyController>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(BLE_SERVICES_MODEL)

enum {
    ServiceNameRole = Qt::UserRole + 1,
    ServiceUuidRole
};

ServicesModel::ServicesModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

bool ServicesModel::isRunning() const
{
    return m_running;
}

void ServicesModel::setRunning(bool running)
{
    if (m_running == running)
        return;
    m_running = running;
    qCDebug(BLE_SERVICES_MODEL) << "Set running:" << m_running;
    emit runningChanged(m_running);
}

bool ServicesModel::isConnected() const
{
    return m_connected;
}

void ServicesModel::setConnected(bool connected)
{
    if (m_connected == connected)
        return;
    m_connected = connected;
    qCDebug(BLE_SERVICES_MODEL) << "Set connected:" << m_connected;
    emit connectedChanged(m_connected);
}

QString ServicesModel::errorString() const
{
    return m_controller ? m_controller->errorString()
                        : tr("No controller object set");
}

void ServicesModel::update(const QString &deviceAddress)
{
    if (m_running)
        return;
    if (m_controller && (m_controller->remoteAddress()
            == QBluetoothAddress(deviceAddress))) {
        return;
    }

    qCDebug(BLE_SERVICES_MODEL) << "Start services discovery";
    setRunning(true);

    beginResetModel();
    delete m_controller;
    m_controller = new QLowEnergyController(QBluetoothAddress(deviceAddress),
                                            this);
    qDeleteAll(m_services);
    m_services.clear();
    endResetModel();

    setConnected(false);

    connect(m_controller, &QLowEnergyController::stateChanged,
            [this](QLowEnergyController::ControllerState state) {
        switch (state) {
        case QLowEnergyController::ConnectingState:
        case QLowEnergyController::DiscoveringState:
            setRunning(true);
            break;
        case QLowEnergyController::ConnectedState:
            setConnected(true);
            m_controller->discoverServices();
            break;
        case QLowEnergyController::UnconnectedState:
            //setConnected(false);
            break;
        case QLowEnergyController::DiscoveredState:
            setRunning(false);
            break;
        default:
            break;
        }
    });

    connect(m_controller, QOverload<QLowEnergyController::Error>::of(
                &QLowEnergyController::error),
            [this](QLowEnergyController::Error error) {
        Q_UNUSED(error);
        setRunning(false);
        emit errorOccurred();
    });

    connect(m_controller, &QLowEnergyController::serviceDiscovered,
            [this](const QBluetoothUuid &serviceUuid) {
        const auto serviceEnd = m_services.cend();
        const auto serviceIt = std::find_if(m_services.cbegin(), serviceEnd,
                                            [serviceUuid](const QLowEnergyService *service)
        {
            return service->serviceUuid() == serviceUuid;
        });

        if (serviceIt != serviceEnd) {
            qCWarning(BLE_SERVICES_MODEL) << "Nothing to add, service already is in model:"
                                          << serviceUuid;
            return;
        }

        const auto service = m_controller->createServiceObject(serviceUuid, this);
        if (!service) {
            qCWarning(BLE_SERVICES_MODEL) << "Unable to create service object:"
                                          << serviceUuid;
            return;
        }

        qCDebug(BLE_SERVICES_MODEL) << "Add service:" << serviceUuid;
        const auto rowsCount = m_services.count();
        beginInsertRows(QModelIndex(), rowsCount, rowsCount);
        m_services.append(service);
        endInsertRows();
    });

    m_controller->connectToDevice();
}

QObject *ServicesModel::service(const QString &serviceUuid) const
{
    const auto serviceEnd = m_services.cend();
    const auto serviceIt = std::find_if(m_services.cbegin(), serviceEnd,
                                        [serviceUuid](const QLowEnergyService *service)
    {
        return service->serviceUuid() == QUuid(serviceUuid);
    });

    return (serviceIt != serviceEnd) ? *serviceIt : nullptr;
}

int ServicesModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_services.count();
}

QVariant ServicesModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0)
        return QVariant();
    if (index.row() >= m_services.count())
        return QVariant();

    const auto row = index.row();
    const auto service = m_services.at(row);
    switch (role) {
    case ServiceNameRole:
        return service->serviceName();
    case ServiceUuidRole:
        return service->serviceUuid();
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> ServicesModel::roleNames() const
{
    return {
        { ServiceNameRole, "name" },
        { ServiceUuidRole, "uuid" }
    };
}
