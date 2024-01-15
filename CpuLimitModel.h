/*
 * Copyright (c) 2024 Malek Khlif
 * Licensed under the MIT License
 * Contact: <malek.khlif@outlook.com>
 */

#ifndef CPULIMITMODEL_H
#define CPULIMITMODEL_H


#include <QAbstractTableModel>

/**
 * @brief CpuLimitModel class
 *
 */
class CpuLimitModel final : public QAbstractTableModel
{
    Q_OBJECT

public:

    explicit CpuLimitModel();

    enum CpuLimitModelRoles
    {
        Pid =  Qt::UserRole + 1,
        User,
        CpuUsage,
        CpuLimit,
        Command,
    };

    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex& parent) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
};

#endif // CPULIMITMODEL_H
