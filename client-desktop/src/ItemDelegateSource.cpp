#include "ItemDelegateSource.h"

ZapFR::Client::ItemDelegateSource::ItemDelegateSource(QObject* parent) : QStyledItemDelegate(parent)
{
}

void ZapFR::Client::ItemDelegateSource::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    static bool initDone{false};
    static auto titleTextOptions = QTextOption(Qt::AlignLeft | Qt::AlignVCenter);
    static auto unreadTextOptions = QTextOption(Qt::AlignCenter | Qt::AlignVCenter);
    if (!initDone)
    {
        titleTextOptions.setWrapMode(QTextOption::NoWrap);
        unreadTextOptions.setWrapMode(QTextOption::NoWrap);
        initDone = true;
    }

    painter->setRenderHint(QPainter::Antialiasing);

    // determine colors to use
    auto palette = qobject_cast<QWidget*>(parent())->palette();
    QBrush brushBackground;
    auto brushText = palette.text();
    auto brushUnreadBadgeBackground = palette.highlight();
    auto penUnreadBadgeText = QPen(QBrush(Qt::white), 1.0);
    bool paintBackground{false};
    palette.setCurrentColorGroup((option.state & QStyle::State_Active) == QStyle::State_Active ? QPalette::ColorGroup::Active : QPalette::ColorGroup::Inactive);
    if ((option.state & QStyle::State_Selected) == QStyle::State_Selected)
    {
        paintBackground = true;
        brushBackground = palette.highlight();
        brushText = palette.highlightedText();
        brushUnreadBadgeBackground = QBrush(Qt::white);
        penUnreadBadgeText = QPen(QBrush(Qt::black), 1.0);
    }

    // paint the background
    if (paintBackground)
    {
        painter->fillRect(option.rect, brushBackground);
    }

    // draw the title
    auto title = index.data(Qt::DisplayRole).toString();
    painter->setPen(QPen(brushText, 1.0));
    painter->drawText(option.rect, title, titleTextOptions);

    // draw the unread amount badge
    if (index.data(SourceTreeEntryTypeRole) == SOURCETREE_ENTRY_TYPE_FEED)
    {
        auto unreadCount = index.data(SourceTreeEntryUnreadCount).toULongLong();
        if (unreadCount > 0)
        {
            auto unreadRect = option.rect;
            unreadRect.moveLeft(option.rect.right() - 30);
            unreadRect.moveTop(option.rect.top() + 2);
            unreadRect.setWidth(25);
            unreadRect.setHeight(option.rect.height() - 4);

            QPainterPath path;
            path.addRoundedRect(unreadRect, 6, 6);
            painter->fillPath(path, brushUnreadBadgeBackground);

            painter->setPen(penUnreadBadgeText);
            painter->drawText(unreadRect, QString("%1").arg(unreadCount), unreadTextOptions);
        }
    }
}

QSize ZapFR::Client::ItemDelegateSource::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    auto s = QStyledItemDelegate::sizeHint(option, index);
    s.setHeight(s.height() + 8);
    return s;
}
