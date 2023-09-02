/*
    ZapFeedReader - RSS/Atom feed reader
    Copyright (C) 2023-present  Kasper Nauwelaerts (zapfr at zappatic dot net)

    ZapFeedReader is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    ZapFeedReader is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with ZapFeedReader.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "delegates/ItemDelegateLog.h"
#include "FeedIconCache.h"
#include "Utilities.h"
#include "ZapFR/Log.h"
#include "widgets/TableViewLogs.h"

ZapFR::Client::ItemDelegateLog::ItemDelegateLog(QObject* parent) : QStyledItemDelegate(parent)
{
}

void ZapFR::Client::ItemDelegateLog::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    static bool initDone{false};
    static auto titleTextOptions = QTextOption(Qt::AlignLeft | Qt::AlignVCenter);
    if (!initDone)
    {
        titleTextOptions.setWrapMode(QTextOption::NoWrap);
        initDone = true;
    }

    auto parentTableView = qobject_cast<TableViewLogs*>(parent());

    painter->setRenderHint(QPainter::SmoothPixmapTransform, true);
    painter->setRenderHint(QPainter::TextAntialiasing, true);
    painter->setRenderHint(QPainter::Antialiasing, true);

    // determine colors to use
    auto palette = parentTableView->palette();
    QBrush brushBackground;
    bool paintBackground{false};
    QBrush brushText = palette.text();
    palette.setCurrentColorGroup((option.state & QStyle::State_Active) == QStyle::State_Active ? QPalette::ColorGroup::Active : QPalette::ColorGroup::Inactive);
    if ((option.state & QStyle::State_Selected) == QStyle::State_Selected)
    {
        paintBackground = true;
        brushBackground = palette.highlight();
        brushText = palette.highlightedText();
    }

    // paint the background
    if (paintBackground)
    {
        painter->fillRect(option.rect, brushBackground);
    }

    auto currentColumn = index.column();
    switch (currentColumn)
    {
        case LogsColumnFeed:
        {
            auto feedIDVariant = index.data(LogFeedIDRole);
            if (!feedIDVariant.isNull() && feedIDVariant.isValid())
            {
                auto sourceID = index.data(LogParentSourceIDRole).toULongLong();
                auto pixmap = FeedIconCache::icon(sourceID, feedIDVariant.toULongLong());
                if (!pixmap.isNull())
                {
                    painter->drawPixmap(Utilities::centeredSquareInRectangle(option.rect, 0.6f), pixmap, pixmap.rect());
                }
            }
            break;
        }
        case LogsColumnLogLevel:
        {
            static std::unique_ptr<QPixmap> pixmapDebug{nullptr};
            static std::unique_ptr<QPixmap> pixmapInfo{nullptr};
            static std::unique_ptr<QPixmap> pixmapWarning{nullptr};
            static std::unique_ptr<QPixmap> pixmapError{nullptr};
            if (pixmapDebug == nullptr)
            {
                pixmapDebug = std::make_unique<QPixmap>(":/logLevelDebug.svg");
            }
            if (pixmapInfo == nullptr)
            {
                pixmapInfo = std::make_unique<QPixmap>(":/logLevelInfo.svg");
            }
            if (pixmapWarning == nullptr)
            {
                pixmapWarning = std::make_unique<QPixmap>(":/logLevelWarning.svg");
            }
            if (pixmapError == nullptr)
            {
                pixmapError = std::make_unique<QPixmap>(":/logLevelError.svg");
            }

            QPixmap* pixmap{nullptr};

            auto logLevel = index.data(LogLevelRole).toULongLong();
            switch (logLevel)
            {
                case ZapFR::Engine::LogLevel::Debug:
                {
                    pixmap = pixmapDebug.get();
                    break;
                }
                case ZapFR::Engine::LogLevel::Info:
                {
                    pixmap = pixmapInfo.get();
                    break;
                }
                case ZapFR::Engine::LogLevel::Warning:
                {
                    pixmap = pixmapWarning.get();
                    break;
                }
                case ZapFR::Engine::LogLevel::Error:
                {
                    pixmap = pixmapError.get();
                    break;
                }
            }

            if (pixmap != nullptr)
            {
                auto targetWidth = static_cast<float>(option.rect.height()) * 0.7f;
                auto targetX = static_cast<float>(option.rect.left()) + ((static_cast<float>(option.rect.width()) / 2.0f) - (targetWidth / 2.0f));
                auto targetY = option.rect.top() + ((option.rect.height() / 2.0) - (targetWidth / 2.0f));
                auto target = QRectF(targetX, targetY, targetWidth, targetWidth);
                painter->drawPixmap(target, *pixmap, pixmap->rect());
            }

            break;
        }
        default:
        {
            auto titleRect = option.rect.adjusted(5, 0, -5, 0);
            auto title = index.data(Qt::DisplayRole).toString();
            static auto whitespaceRe = QRegularExpression(R"(\s+)");
            title.replace(whitespaceRe, " ");
            painter->setPen(QPen(brushText, 1.0));
            auto fm = QFontMetrics(painter->font());
            auto elidedTitle = fm.elidedText(title, Qt::ElideRight, titleRect.width());
            painter->drawText(titleRect, elidedTitle, titleTextOptions);
            break;
        }
    }
}
