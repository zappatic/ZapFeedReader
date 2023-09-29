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

#include <QDateTime>
#include <QFile>

#include "Utilities.h"

std::unordered_map<ZapFR::Engine::FlagColor, QPixmap> ZapFR::Client::Utilities::msFilledFlagCache{};
std::unordered_map<ZapFR::Engine::FlagColor, QPixmap> ZapFR::Client::Utilities::msUnfilledFlagCache{};

QString ZapFR::Client::Utilities::prettyDate(const QString& iso8601Date)
{
    auto dateTime = QDateTime::fromString(iso8601Date, Qt::ISODate);
    if (!dateTime.isValid())
    {
        return QObject::tr("Invalid date");
    }
    dateTime = dateTime.toLocalTime();

    auto locale = QLocale();
    auto now = QDateTime::currentDateTime();

    auto nowDate = now.date();
    auto yesterdayDate = nowDate.addDays(-1);

    auto date = dateTime.date();
    auto time = dateTime.time();

    auto datePart = locale.toString(date, QLocale::ShortFormat);

    if (nowDate.year() == date.year() && nowDate.month() == date.month() && nowDate.day() == date.day())
    {
        datePart = QObject::tr("Today,");
    }
    else if (yesterdayDate.year() == date.year() && yesterdayDate.month() == date.month() && yesterdayDate.day() == date.day())
    {
        datePart = QObject::tr("Yesterday,");
    }

    auto timePart = locale.toString(time, QLocale::ShortFormat);
    return datePart + " " + timePart;
}

QRectF ZapFR::Client::Utilities::centeredSquareInRectangle(const QRectF& sourceRect, float heightFactor)
{
    auto targetWidth = static_cast<float>(sourceRect.height()) * heightFactor;
    auto targetX = static_cast<float>(sourceRect.left()) + ((static_cast<float>(sourceRect.width()) / 2.0f) - (targetWidth / 2.0f));
    auto targetY = sourceRect.top() + ((sourceRect.height() / 2.0) - (targetWidth / 2.0f));
    return QRectF(targetX, targetY, targetWidth, targetWidth);
}

const QPixmap& ZapFR::Client::Utilities::flag(ZapFR::Engine::FlagColor color, FlagStyle flagStyle)
{
    switch (flagStyle)
    {
        case FlagStyle::Filled:
        {
            if (msFilledFlagCache.contains(color))
            {
                return msFilledFlagCache.at(color);
            }
            break;
        }
        case FlagStyle::Unfilled:
        {
            if (msUnfilledFlagCache.contains(color))
            {
                return msUnfilledFlagCache.at(color);
            }
            break;
        }
    }

    auto [red, green, blue] = ZapFR::Engine::Flag::rgbForColor(color);
    QColor c(red, green, blue);

    auto svgFile = QFile(flagStyle == FlagStyle::Filled ? ":/flagFilled.svg" : ":/flagUnfilled.svg");
    svgFile.open(QIODeviceBase::ReadOnly);
    auto svgContents = QString(svgFile.readAll());
    svgFile.close();
    svgContents.replace("{#color}", c.name());

    QImage img;
    img.loadFromData(svgContents.toUtf8());
    auto scaledFlag = img.scaled(50, 50, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    auto pixmap = QPixmap::fromImage(scaledFlag);
    switch (flagStyle)
    {
        case FlagStyle::Filled:
        {
            msFilledFlagCache[color] = pixmap;
            return msFilledFlagCache.at(color);
        }
        case FlagStyle::Unfilled:
        default:
        {
            msUnfilledFlagCache[color] = pixmap;
            return msUnfilledFlagCache.at(color);
        }
    }
}
