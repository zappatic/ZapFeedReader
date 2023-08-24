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

#include "widgets/SearchWidget.h"
#include "ui_SearchWidget.h"

ZapFR::Client::SearchWidget::SearchWidget(QWidget* parent) : QWidget(parent), ui(new Ui::SearchWidget)
{
    ui->setupUi(this);

    connect(ui->lineEditSearch, &QLineEdit::returnPressed, [&]() { emit searchRequested(); });

    for (const auto& action : ui->lineEditSearch->findChildren<QAction*>())
    {
        connect(action, &QAction::triggered,
                [&]()
                {
                    // we have to do this on the next event processing cycle, because at this point
                    // the previous text is still in the line edit
                    QTimer::singleShot(0, [&]() { emit searchRequested(); });
                });
        break;
    }

    mSearchIconAction = ui->lineEditSearch->addAction(QIcon(), QLineEdit::LeadingPosition);
}

ZapFR::Client::SearchWidget::~SearchWidget()
{
    delete ui;
}

QString ZapFR::Client::SearchWidget::searchQuery() const
{
    return ui->lineEditSearch->text();
}

void ZapFR::Client::SearchWidget::setSearchIconColor(const QString& color)
{
    static QString svgContents{};
    if (svgContents.isEmpty())
    {
        auto svgFile = QFile(":/search.svg");
        svgFile.open(QIODeviceBase::ReadOnly);
        svgContents = QString(svgFile.readAll());
        svgFile.close();
    }

    QImage img;
    auto svg = svgContents;
    svg.replace("{#color}", color);
    img.loadFromData(svg.toUtf8());

    QIcon icon;
    icon.addPixmap(QPixmap::fromImage(img), QIcon::Normal, QIcon::On);

    mSearchIconAction->setIcon(icon);
}