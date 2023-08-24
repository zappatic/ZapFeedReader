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

#include "widgets/WidgetPostCaption.h"

ZapFR::Client::WidgetPostCaption::WidgetPostCaption(QWidget* parent) : QWidget(parent)
{
}

void ZapFR::Client::WidgetPostCaption::setCaption(const QString& caption)
{
    mCaption = caption;
}

void ZapFR::Client::WidgetPostCaption::paintEvent(QPaintEvent* /*event*/)
{
    if (!mCaption.isEmpty())
    {
        auto currentColorScheme = QGuiApplication::styleHints()->colorScheme();
        auto textPen = QPen(currentColorScheme == Qt::ColorScheme::Dark ? QColor(68, 68, 68) : QColor(170, 170, 170));

        auto textRect = QRect(0, 50, width(), 100);
        auto f = font();
        f.setPixelSize(32);
        f.setWeight(QFont::DemiBold);

        auto painter = QPainter(this);
        painter.setPen(textPen);
        painter.setFont(f);
        painter.drawText(textRect, Qt::AlignTop | Qt::AlignHCenter, mCaption);
    }
}
