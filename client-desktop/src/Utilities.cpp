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

#include "Utilities.h"

QString ZapFR::Client::Utilities::prettyDate(const QString& iso8601Date)
{
    auto dateTime = QDateTime::fromString(iso8601Date, Qt::ISODate);
    if (!dateTime.isValid())
    {
        return QObject::tr("Invalid date");
    }

    auto locale = QLocale();
    auto now = QDateTime::currentDateTime();

    auto nowDate = now.date();
    auto yesterdayDate = nowDate.addDays(-1);

    auto date = dateTime.date();
    auto time = dateTime.time();

    auto datePart = locale.toString(date, QLocale::ShortFormat);

    if (nowDate.year() == date.year() && nowDate.month() == date.month())
    {
        if (nowDate.day() == date.day())
        {
            datePart = QObject::tr("Today,");
        }
        else if (yesterdayDate.day() == date.day())
        {
            datePart = QObject::tr("Yesterday,");
        }
    }

    auto timePart = locale.toString(time, QLocale::ShortFormat);
    return datePart + " " + timePart;
}
