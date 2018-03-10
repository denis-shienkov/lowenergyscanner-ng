#include "descriptorsmodel.h"

#include <QLowEnergyService>
#include <QLowEnergyCharacteristic>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(BLE_DESCRIPTORS_MODEL)

enum {
    CharacteristicNameRole = Qt::UserRole + 1,
    CharacteristicUuidRole,
    CharacteristicValueRole
};

DescriptorsModel::DescriptorsModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

void DescriptorsModel::update(QObject *service, const QString &characteristicUuid)
{
    qCDebug(BLE_DESCRIPTORS_MODEL) << "Start descriptors discovery";

    beginResetModel();
    m_descriptors.clear();
    endResetModel();

    const auto characteristics = qobject_cast<QLowEnergyService *>(service)->characteristics();
    const auto characteristicEnd = characteristics.cend();
    const auto characteristicIt = std::find_if(characteristics.cbegin(), characteristicEnd,
                                               [characteristicUuid](const QLowEnergyCharacteristic &characteristic) {
        return characteristic.uuid() == QBluetoothUuid(characteristicUuid);
    });
    if (characteristicIt != characteristicEnd) {
        const auto descriptors = characteristicIt->descriptors();
        for (const auto &descriptor : descriptors) {
            if (m_descriptors.contains(descriptor)) {
                qCWarning(BLE_DESCRIPTORS_MODEL) << "Nothing to add, descriptor already is in model:"
                                                 << descriptor.uuid();
                continue;
            }

            qCDebug(BLE_DESCRIPTORS_MODEL) << "Add descriptor:" << descriptor.uuid();
            const auto rowsCount = m_descriptors.count();
            beginInsertRows(QModelIndex(), rowsCount, rowsCount);
            m_descriptors.append(descriptor);
            endInsertRows();
        }
    }
}

int DescriptorsModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_descriptors.count();
}

QVariant DescriptorsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0)
        return QVariant();

    if (index.row() >= m_descriptors.count())
        return QVariant();

    const auto row = index.row();
    const auto &descriptor = m_descriptors.at(row);

    switch (role) {
    case CharacteristicNameRole:
        return descriptor.name();
    case CharacteristicUuidRole:
        return descriptor.uuid();
    case CharacteristicValueRole:
        return descriptor.value().toHex();
    default:
        break;
    }

    return QVariant();
}

QHash<int, QByteArray> DescriptorsModel::roleNames() const
{
    return {
        { CharacteristicNameRole, "name" },
        { CharacteristicUuidRole, "uuid" },
        { CharacteristicValueRole, "value" }
    };
}
