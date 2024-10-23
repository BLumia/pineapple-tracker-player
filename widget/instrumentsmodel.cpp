#include "instrumentsmodel.h"

#include "player.h"

InstrumentsModel::InstrumentsModel(Player * player, QObject *parent)
    : QStringListModel(parent)
    , m_player(player)
{

}

Qt::ItemFlags InstrumentsModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = QStringListModel::flags(index);
    if (index.isValid()) {
        flags.setFlag(Qt::ItemIsUserCheckable);
        flags.setFlag(Qt::ItemNeverHasChildren);
        flags.setFlag(Qt::ItemIsEditable, false);
    }
    return flags;
}

bool InstrumentsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || role != Qt::CheckStateRole) return false;

    if (value == Qt::Checked) {
        m_mutedInstruments.remove(index);
        m_player->setInstrumentMuteStatus(index.row(), false);
    } else {
        m_mutedInstruments.insert(index);
        m_player->setInstrumentMuteStatus(index.row(), true);
    }

    emit dataChanged(index, index);
    return true;
}


QVariant InstrumentsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) return {};

    if (role == Qt::CheckStateRole) return m_mutedInstruments.contains(index) ? Qt::Unchecked : Qt::Checked;

    return QStringListModel::data(index, role);
}
