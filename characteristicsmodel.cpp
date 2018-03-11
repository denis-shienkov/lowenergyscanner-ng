#include "characteristicsmodel.h"

#include <QLowEnergyService>
#include <QLowEnergyCharacteristic>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(BLE_CHARACTERISTICS_MODEL)

enum {
    CharacteristicNameRole = Qt::UserRole + 1,
    CharacteristicUuidRole,
    CharacteristicPropertiesRole,
    CharacteristicWritableRole,
    CharacteristicReadableRole,
    CharacteristicNotifyableRole,
    CharacteristicIndicatableRole,
    CharacteristicNotificationEnabledRole,
    CharacteristicIndicationEnabledRole,
    CharacteristicValueRole
};

static QString decodeProperties(QLowEnergyCharacteristic::PropertyTypes pt)
{
    QStringList properties;
    if (pt == QLowEnergyCharacteristic::Unknown) {
        properties << CharacteriticsModel::tr("Unknown");
    } else {
        if (pt & QLowEnergyCharacteristic::Broadcasting)
            properties << CharacteriticsModel::tr("Broadcasting");
        if (pt & QLowEnergyCharacteristic::Read)
            properties << CharacteriticsModel::tr("Read");
        if (pt & QLowEnergyCharacteristic::WriteNoResponse)
            properties << CharacteriticsModel::tr("WriteNoResponse");
        if (pt & QLowEnergyCharacteristic::Write)
            properties << CharacteriticsModel::tr("Write");
        if (pt & QLowEnergyCharacteristic::Notify)
            properties << CharacteriticsModel::tr("Notify");
        if (pt & QLowEnergyCharacteristic::Indicate)
            properties << CharacteriticsModel::tr("Indicate");
        if (pt & QLowEnergyCharacteristic::WriteSigned)
            properties << CharacteriticsModel::tr("WriteSigned");
        if (pt & QLowEnergyCharacteristic::ExtendedProperty)
            properties << CharacteriticsModel::tr("ExtendedProperty");
    }

    return properties.join(",");
}

CharacteriticsModel::CharacteriticsModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

bool CharacteriticsModel::isRunning() const
{
    return m_running;
}

void CharacteriticsModel::setRunning(bool running)
{
    if (m_running == running)
        return;
    m_running = running;
    qCDebug(BLE_CHARACTERISTICS_MODEL) << "Set running:" << m_running;
    emit runningChanged(m_running);
}

QString CharacteriticsModel::errorString() const
{
    return m_service ? tr("Service I/O error") : tr("No service object set");
}

void CharacteriticsModel::update(QObject *service)
{
    if (m_running)
        return;

    if (m_service == service)
        return;

    qCDebug(BLE_CHARACTERISTICS_MODEL) << "Start characteristics discovery";

    if (m_service) {
        disconnect(m_service);
    }

    beginResetModel();
    m_service = qobject_cast<QLowEnergyService *>(service);
    m_characteristicUuids.clear();
    endResetModel();

    setRunning(false);

    if (m_service) {
        connect(m_service, &QLowEnergyService::stateChanged,
                [this](QLowEnergyService::ServiceState state) {
            switch (state) {
            case QLowEnergyService::DiscoveringServices:
                setRunning(true);
                break;
            case QLowEnergyService::ServiceDiscovered:
                updateCharacteristics();
                setRunning(false);
                break;
            default:
                break;
            }

        });

        connect(m_service, QOverload<QLowEnergyService::ServiceError>::of(&QLowEnergyService::error),
                [this](QLowEnergyService::ServiceError error) {
            Q_UNUSED(error);
            setRunning(false);
            if (error != QLowEnergyService::DescriptorReadError)
                emit errorOccurred();

            // Update whole model data.
            const auto topLeftModelIndex = index(0, 0);
            const auto bottomRightModelIndex = index(m_characteristicUuids.count() - 1, 0);
            emit dataChanged(topLeftModelIndex, bottomRightModelIndex);
        });

        connect(m_service, &QLowEnergyService::characteristicRead,
                [this](const QLowEnergyCharacteristic &characteristic, const QByteArray &value) {
            Q_UNUSED(value);
            const auto characteristicUuid = characteristic.uuid();
            qCDebug(BLE_CHARACTERISTICS_MODEL) << "Characteristic read completed:"
                                               << characteristicUuid
                                               << value.toHex();
            const auto row = m_characteristicUuids.indexOf(characteristicUuid);
            const auto modelIndex = index(row, 0);
            emit dataChanged(modelIndex, modelIndex);
        });

        connect(m_service, &QLowEnergyService::characteristicWritten,
                [this](const QLowEnergyCharacteristic &characteristic, const QByteArray &value) {
            Q_UNUSED(value);
            const auto characteristicUuid = characteristic.uuid();
            qCDebug(BLE_CHARACTERISTICS_MODEL) << "Characteristic write completed:"
                                               << characteristicUuid
                                               << value.toHex();
            const auto row = m_characteristicUuids.indexOf(characteristicUuid);
            const auto modelIndex = index(row, 0);
            emit dataChanged(modelIndex, modelIndex);
        });

        connect(m_service, &QLowEnergyService::characteristicChanged,
                [this](const QLowEnergyCharacteristic &characteristic, const QByteArray &value) {
            Q_UNUSED(value);
            const auto characteristicUuid = characteristic.uuid();
            qCDebug(BLE_CHARACTERISTICS_MODEL) << "Characteristic change completed:"
                                               << characteristicUuid
                                               << value.toHex();
            const auto row = m_characteristicUuids.indexOf(characteristicUuid);
            const auto modelIndex = index(row, 0);
            emit dataChanged(modelIndex, modelIndex);
        });

        connect(m_service, &QLowEnergyService::descriptorWritten,
                [this](const QLowEnergyDescriptor &descriptor, const QByteArray &value) {
            Q_UNUSED(value);
            qCDebug(BLE_CHARACTERISTICS_MODEL) << "Write descriptor completed:"
                                               << descriptor.uuid()
                                               << value.toHex();
            for (auto characteristicUuidIt = m_characteristicUuids.cbegin();
                 characteristicUuidIt < m_characteristicUuids.cend(); ++characteristicUuidIt) {
                const auto &characteristic = m_service->characteristic(*characteristicUuidIt);
                const auto &descriptors = characteristic.descriptors();
                if (!descriptors.contains(descriptor))
                    continue;
                const auto row = std::distance(m_characteristicUuids.cbegin(), characteristicUuidIt);
                const auto modelIndex = index(row, 0);
                emit dataChanged(modelIndex, modelIndex);
            }
        });

        if (m_service->state() == QLowEnergyService::ServiceDiscovered) {
            updateCharacteristics();
            setRunning(false);
        } else if (m_service->state() == QLowEnergyService::DiscoveryRequired) {
            m_service->discoverDetails();
        }
    }
}

void CharacteriticsModel::read(const QString &characteristicUuid)
{
    const auto &characteristic = m_service->characteristic(
                QBluetoothUuid(characteristicUuid));
    if (!characteristic.isValid())
        return;

    qCDebug(BLE_CHARACTERISTICS_MODEL) << "Start read characteristic:"
                                       << characteristicUuid;

    m_service->readCharacteristic(characteristic);
}

void CharacteriticsModel::write(const QString &characteristicUuid,
                                const QByteArray &hexValue)
{
    const auto &characteristic = m_service->characteristic(
                QBluetoothUuid(characteristicUuid));
    if (!characteristic.isValid())
        return;

    qCDebug(BLE_CHARACTERISTICS_MODEL) << "Start write characteristic:"
                                       << characteristicUuid
                                       << hexValue;

    m_service->writeCharacteristic(characteristic, QByteArray::fromHex(hexValue));
}

void CharacteriticsModel::enableNotification(const QString &characteristicUuid,
                                             bool enable)
{
    const auto &characteristic = m_service->characteristic(
                QBluetoothUuid(characteristicUuid));
    if (!characteristic.isValid())
        return;

    const auto &configDescriptor = characteristic.descriptor(
                QBluetoothUuid::ClientCharacteristicConfiguration);
    if (!configDescriptor.isValid())
        return;

    const auto value = enable ? QByteArray::fromHex("0100")
                              : QByteArray::fromHex("0000");

    qCDebug(BLE_CHARACTERISTICS_MODEL) << "Start write descriptor:"
                                       << configDescriptor.uuid()
                                       << value.toHex();

    m_service->writeDescriptor(configDescriptor, value);
}

void CharacteriticsModel::enableIndication(const QString &characteristicUuid,
                                           bool enable)
{
    const auto &characteristic = m_service->characteristic(
                QBluetoothUuid(characteristicUuid));
    if (!characteristic.isValid())
        return;

    const auto &configDescriptor = characteristic.descriptor(
                QBluetoothUuid::ClientCharacteristicConfiguration);
    if (!configDescriptor.isValid())
        return;

    const auto value = enable ? QByteArray::fromHex("0200")
                              : QByteArray::fromHex("0000");

    qCDebug(BLE_CHARACTERISTICS_MODEL) << "Start write descriptor:"
                                       << configDescriptor.uuid()
                                       << value.toHex();

    m_service->writeDescriptor(configDescriptor, value);
}

QObject *CharacteriticsModel::service() const
{
    return m_service;
}

void CharacteriticsModel::updateCharacteristics()
{
    const auto characteristics = m_service->characteristics();
    for (const auto &characteristic : characteristics) {
        const auto characteristicUuid = characteristic.uuid();
        if (m_characteristicUuids.contains(characteristicUuid)) {
            qCWarning(BLE_CHARACTERISTICS_MODEL) << "Nothing to add, characteristic already is in model:"
                                                 << characteristicUuid;
            continue;
        }

        qCDebug(BLE_CHARACTERISTICS_MODEL) << "Add characteristic:" << characteristicUuid;
        const auto rowsCount = m_characteristicUuids.count();
        beginInsertRows(QModelIndex(), rowsCount, rowsCount);
        m_characteristicUuids.append(characteristicUuid);
        endInsertRows();
    }
}

int CharacteriticsModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_characteristicUuids.count();
}

QVariant CharacteriticsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0)
        return QVariant();
    if (index.row() >= m_characteristicUuids.count())
        return QVariant();

    const auto row = index.row();
    const auto characteristicUuid = m_characteristicUuids.at(row);
    const auto characteristic = m_service->characteristic(characteristicUuid);
    if (!characteristic.isValid())
        return QVariant();

    const auto properties = characteristic.properties();
    const auto configDescriptor = characteristic.descriptor(
                QBluetoothUuid::ClientCharacteristicConfiguration);

    switch (role) {
    case CharacteristicNameRole:
        return characteristic.name();
    case CharacteristicUuidRole:
        return characteristic.uuid();
    case CharacteristicPropertiesRole:
        return decodeProperties(properties);
    case CharacteristicWritableRole:
        return bool(properties & (QLowEnergyCharacteristic::Write
                                  | QLowEnergyCharacteristic::WriteNoResponse
                                  | QLowEnergyCharacteristic::WriteSigned));
    case CharacteristicReadableRole:
        return bool(properties & (QLowEnergyCharacteristic::Read));
    case CharacteristicNotifyableRole:
        return bool(properties & (QLowEnergyCharacteristic::Notify));
    case CharacteristicIndicatableRole:
        return bool(properties & (QLowEnergyCharacteristic::Indicate));
    case CharacteristicNotificationEnabledRole:
        return configDescriptor.isValid()
                && configDescriptor.value() == QByteArray::fromHex("0100");
    case CharacteristicIndicationEnabledRole:
        return configDescriptor.isValid()
                && configDescriptor.value() == QByteArray::fromHex("0200");
    case CharacteristicValueRole:
        return characteristic.value().toHex();
    default:
        break;
    }

    return QVariant();
}

QHash<int, QByteArray> CharacteriticsModel::roleNames() const
{
    return {
        { CharacteristicNameRole, "name" },
        { CharacteristicUuidRole, "uuid" },
        { CharacteristicPropertiesRole, "props" },
        { CharacteristicWritableRole, "writable" },
        { CharacteristicReadableRole, "readable" },
        { CharacteristicNotifyableRole, "notifyable" },
        { CharacteristicIndicatableRole, "indicatable" },
        { CharacteristicNotificationEnabledRole, "notificationEnabled" },
        { CharacteristicIndicationEnabledRole, "indicationEnabled" },
        { CharacteristicValueRole, "value" }
    };
}
