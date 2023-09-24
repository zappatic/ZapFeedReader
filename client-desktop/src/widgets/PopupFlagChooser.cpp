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

#include "widgets/PopupFlagChooser.h"
#include "ui_PopupFlagChooser.h"

ZapFR::Client::PopupFlagChooser::PopupFlagChooser(QWidget* parent) : QWidget(parent, Qt::Window | Qt::FramelessWindowHint), ui(new Ui::PopupFlagChooser)
{
    ui->setupUi(this);
    mFlags.emplace_back(ui->flagBlue);
    mFlags.emplace_back(ui->flagGreen);
    mFlags.emplace_back(ui->flagYellow);
    mFlags.emplace_back(ui->flagOrange);
    mFlags.emplace_back(ui->flagRed);
    mFlags.emplace_back(ui->flagPurple);
    ui->flagBlue->setFlagColor(ZapFR::Engine::FlagColor::Blue);
    ui->flagGreen->setFlagColor(ZapFR::Engine::FlagColor::Green);
    ui->flagYellow->setFlagColor(ZapFR::Engine::FlagColor::Yellow);
    ui->flagOrange->setFlagColor(ZapFR::Engine::FlagColor::Orange);
    ui->flagRed->setFlagColor(ZapFR::Engine::FlagColor::Red);
    ui->flagPurple->setFlagColor(ZapFR::Engine::FlagColor::Purple);

    for (const auto& flag : mFlags)
    {
        connect(flag, &PopupFlag::flagClicked,
                [&](PopupFlag* clickedFlag)
                {
                    clickedFlag->setFlagStyle((clickedFlag->flagStyle() == Utilities::FlagStyle::Filled) ? Utilities::FlagStyle::Unfilled : Utilities::FlagStyle::Filled);
                    emit flagToggled(clickedFlag->flagColor(), clickedFlag->flagStyle());
                });
    }
}

ZapFR::Client::PopupFlagChooser::~PopupFlagChooser()
{
    delete ui;
}

void ZapFR::Client::PopupFlagChooser::focusOutEvent(QFocusEvent* /*event*/)
{
    hide();
}

void ZapFR::Client::PopupFlagChooser::showWithSelectedColors(const QList<QVariant> flagColors)
{
    for (const auto& flag : mFlags)
    {
        flag->setFlagStyle(Utilities::FlagStyle::Unfilled);
    }
    for (const auto& flagColorVariant : flagColors)
    {
        auto flagColor = static_cast<ZapFR::Engine::FlagColor>(flagColorVariant.toUInt());
        for (const auto& flag : mFlags)
        {
            if (flag->flagColor() == flagColor)
            {
                flag->setFlagStyle(Utilities::FlagStyle::Filled);
            }
        }
    }
    show();
    activateWindow();
    raise();
    setFocus();
}

ZapFR::Client::PopupFlag::PopupFlag(QWidget* parent) : QWidget(parent)
{
    setCursor(Qt::PointingHandCursor);
}

void ZapFR::Client::PopupFlag::setFlagColor(ZapFR::Engine::FlagColor flagColor)
{
    mFlagColor = flagColor;
    update();
}

ZapFR::Engine::FlagColor ZapFR::Client::PopupFlag::flagColor() const noexcept
{
    return mFlagColor;
}

void ZapFR::Client::PopupFlag::setFlagStyle(Utilities::FlagStyle flagStyle)
{
    mFlagStyle = flagStyle;
    update();
}

ZapFR::Client::Utilities::FlagStyle ZapFR::Client::PopupFlag::flagStyle() const noexcept
{
    return mFlagStyle;
}

void ZapFR::Client::PopupFlag::paintEvent(QPaintEvent* /*event*/)
{
    auto painter = QPainter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    painter.setRenderHint(QPainter::Antialiasing, true);

    auto g = geometry();
    auto targetRect = QRect(0, 0, g.width(), g.height());
    auto flag = Utilities::flag(mFlagColor, mFlagStyle);
    painter.drawPixmap(Utilities::centeredSquareInRectangle(targetRect, 0.75f), flag, flag.rect());
}

void ZapFR::Client::PopupFlag::mouseReleaseEvent(QMouseEvent* /*event*/)
{
    emit flagClicked(this);
}

void ZapFR::Client::PopupFlag::toggleStyle()
{
    if (mFlagStyle == Utilities::FlagStyle::Unfilled)
    {
        mFlagStyle = Utilities::FlagStyle::Filled;
    }
    else
    {
        mFlagStyle = Utilities::FlagStyle::Unfilled;
    }
    update();
}
