#include "ItemDelegateSource.h"
#include <QGuiApplication>
#include <QPainter>
#include <QPalette>

ZapFR::Client::ItemDelegateSource::ItemDelegateSource(QObject* parent) : QStyledItemDelegate(parent)
{
}

void ZapFR::Client::ItemDelegateSource::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    auto palette = qobject_cast<QWidget*>(parent())->palette();
    auto title = index.data(Qt::DisplayRole).toString();
    static auto titleTextOptions = QTextOption(Qt::AlignLeft | Qt::AlignVCenter);
    titleTextOptions.setWrapMode(QTextOption::NoWrap);

    QBrush brushBackground;
    QBrush brushText = palette.text();
    bool paintBackground{false};

    palette.setCurrentColorGroup((option.state & QStyle::State_Active) == QStyle::State_Active ? QPalette::ColorGroup::Active : QPalette::ColorGroup::Inactive);
    if ((option.state & QStyle::State_Selected) == QStyle::State_Selected)
    {
        paintBackground = true;
        brushBackground = palette.highlight();
        brushText = palette.highlightedText();
    }

    if (paintBackground)
    {
        painter->fillRect(option.rect, brushBackground);
    }

    painter->setPen(QPen(brushText, 1.0));
    painter->drawText(option.rect, title, titleTextOptions);
}

QSize ZapFR::Client::ItemDelegateSource::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    auto s = QStyledItemDelegate::sizeHint(option, index);
    s.setHeight(s.height() + 8);
    return s;
}
