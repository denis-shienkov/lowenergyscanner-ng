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
    CharacteristicNotificationsEnabledRole,
    CharacteristicIndicationsEnabledRole,
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
    m_characteristics.clear();
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
        });

        connect(m_service, &QLowEnergyService::characteristicRead,
                [this](const QLowEnergyCharacteristic &characteristic, const QByteArray &value) {
            Q_UNUSED(value);
            qCDebug(BLE_CHARACTERISTICS_MODEL) << "Finish read characteristic:"
                                               << characteristic.uuid()
                                               << value.toHex();
            const auto row = m_characteristics.indexOf(characteristic);
            m_characteristics[row] = characteristic;
            const auto modelIndex = index(row, 0);
            emit dataChanged(modelIndex, modelIndex);
        });

        connect(m_service, &QLowEnergyService::characteristicWritten,
                [this](const QLowEnergyCharacteristic &characteristic, const QByteArray &value) {
            Q_UNUSED(value);
            qCDebug(BLE_CHARACTERISTICS_MODEL) << "Finish write characteristic:"
                                               << characteristic.uuid()
                                               << value.toHex();
            const auto row = m_characteristics.indexOf(characteristic);
            m_characteristics[row] = characteristic;
            const auto modelIndex = index(row, 0);
            emit dataChanged(modelIndex, modelIndex);
        });

        connect(m_service, &QLowEnergyService::descriptorWritten,
                [this](const QLowEnergyDescriptor &descriptor, const QByteArray &value) {
            Q_UNUSED(value);
            qCDebug(BLE_CHARACTERISTICS_MODEL) << "Finish write descriptor:"
                                               << descriptor.uuid()
                                               << value.toHex();
            for (auto characteristicIt = m_characteristics.begin();
                 characteristicIt < m_characteristics.end(); ++characteristicIt) {
                const auto descriptors = characteristicIt->descriptors();
                if (!descriptors.contains(descriptor))
                    continue;
                *characteristicIt = m_service->characteristic(characteristicIt->uuid());
                const auto row = std::distance(m_characteristics.begin(), characteristicIt);
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
    qCDebug(BLE_CHARACTERISTICS_MODEL) << "Start read characteristic:"
                                       << characteristicUuid;

    const auto characteristicEnd = m_characteristics.cend();
    const auto characteristicIt = std::find_if(m_characteristics.cbegin(), characteristicEnd,
                                               [characteristicUuid](
                                               const QLowEnergyCharacteristic &characteristic) {
        return characteristic.uuid() == QBluetoothUuid(characteristicUuid);
    });
    if (characteristicIt != characteristicEnd)
        m_service->readCharacteristic(*characteristicIt);
}

void CharacteriticsModel::write(const QString &characteristicUuid,
                                const QByteArray &hexValue)
{
    qCDebug(BLE_CHARACTERISTICS_MODEL) << "Start write characteristic:"
                                       << characteristicUuid
                                       << hexValue;

    const auto characteristicEnd = m_characteristics.cend();
    const auto characteristicIt = std::find_if(m_characteristics.cbegin(), characteristicEnd,
                                               [characteristicUuid](
                                               const QLowEnergyCharacteristic &characteristic) {
        return characteristic.uuid() == QBluetoothUuid(characteristicUuid);
    });
    if (characteristicIt != characteristicEnd)
        m_service->writeCharacteristic(*characteristicIt, QByteArray::fromHex(hexValue));
}

void CharacteriticsModel::enableNotifications(const QString &characteristicUuid,
                                              bool enable)
{
    qCDebug(BLE_CHARACTERISTICS_MODEL) << "Start notification enable for config descriptor of characteristic:"
                                       << characteristicUuid
                                       << enable;

    const auto characteristicEnd = m_characteristics.cend();
    const auto characteristicIt = std::find_if(m_characteristics.cbegin(), characteristicEnd,
                                               [characteristicUuid](
                                               const QLowEnergyCharacteristic &characteristic) {
        return characteristic.uuid() == QBluetoothUuid(characteristicUuid);
    });
    if (characteristicIt != characteristicEnd) {
        const auto configDescriptor = characteristicIt->descriptor(
                    QBluetoothUuid::ClientCharacteristicConfiguration);
        if (configDescriptor.isValid()) {
            m_service->writeDescriptor(configDescriptor, enable ? QByteArray::fromHex("0100")
                                                                : QByteArray::fromHex("0000"));
        }
    }
}

QObject *CharacteriticsModel::service() const
{
    return m_service;
}

void CharacteriticsModel::updateCharacteristics()
{
    const auto characteristics = m_service->characteristics();
    for (const auto &characteristic : characteristics) {
        if (m_characteristics.contains(characteristic)) {
            qCWarning(BLE_CHARACTERISTICS_MODEL) << "Nothing to add, characteristic already is in model:"
                                                 << characteristic.uuid();
            continue;
        }

        qCDebug(BLE_CHARACTERISTICS_MODEL) << "Add characteristic:" << characteristic.uuid();
        const auto rowsCount = m_characteristics.count();
        beginInsertRows(QModelIndex(), rowsCount, rowsCount);
        m_characteristics.append(characteristic);
        endInsertRows();
    }
}

int CharacteriticsModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_characteristics.count();
}

QVariant CharacteriticsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0)
        return QVariant();

    if (index.row() >= m_characteristics.count())
        return QVariant();

    const auto row = index.row();
    const auto characteristic = m_characteristics.at(row);
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
    case CharacteristicNotificationsEnabledRole: {
        const auto value = configDescriptor.value();
        qCDebug(BLE_CHARACTERISTICS_MODEL) << "CH:" << characteristic.uuid() << "V:" << value.toHex();
        return configDescriptor.isValid() && value == QByteArray::fromHex("0100");
    }
    case CharacteristicIndicationsEnabledRole:
        return configDescriptor.isValid() && configDescriptor.value() == QByteArray::fromHex("0200");
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
        { CharacteristicNotificationsEnabledRole, "notificationEnabled" },
        { CharacteristicIndicationsEnabledRole, "indicationEnabled" },
        { CharacteristicValueRole, "value" }
    };
}
