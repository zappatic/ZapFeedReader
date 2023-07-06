#include "ItemDelegateSource.h"
#include <QPainter>

ZapFR::Client::ItemDelegateSource::ItemDelegateSource(QObject* parent) : QStyledItemDelegate(parent)
{
}

void ZapFR::Client::ItemDelegateSource::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    static const auto paddingLeft = 2;

    QRect r = option.rect;

    auto title = index.data(Qt::DisplayRole).toString();

    painter->drawText(r.x() + paddingLeft, r.y() + 10 + 2, title);
}
