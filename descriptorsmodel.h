#ifndef DESCRIPTORSMODEL_H
#define DESCRIPTORSMODEL_H

#include <QLowEnergyDescriptor>
#include <QAbstractListModel>

class DescriptorsModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit DescriptorsModel(QObject *parent = nullptr);

    Q_INVOKABLE void update(QObject *service, const QString &characteristicUuid);

private:
    int rowCount(const QModelIndex &parent) const final;
    QVariant data(const QModelIndex &index, int role) const final;
    QHash<int, QByteArray> roleNames() const final;

    QVector<QLowEnergyDescriptor> m_descriptors;
};

#endif // DESCRIPTORSMODEL_H
