#pragma once

#include <QStringListModel>

class Player;
class InstrumentsModel : public QStringListModel
{
public:
    explicit InstrumentsModel(Player * player = nullptr, QObject *parent = nullptr);

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    QVariant data(const QModelIndex &index, int role) const override;

private:
    QSet<QPersistentModelIndex> m_mutedInstruments;
    Player * m_player = nullptr;
};
