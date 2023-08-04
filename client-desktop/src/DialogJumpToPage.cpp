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

#include "DialogJumpToPage.h"
#include "ui_DialogJumpToPage.h"

ZapFR::Client::DialogJumpToPage::DialogJumpToPage(QWidget* parent) : QDialog(parent), ui(new Ui::DialogJumpToPage)
{
    ui->setupUi(this);

    for (const auto& button : ui->buttonBox->buttons())
    {
        if (ui->buttonBox->buttonRole(button) == QDialogButtonBox::ButtonRole::AcceptRole)
        {
            button->setText(tr("Jump"));
            break;
        }
    }

    connect(ui->lineEditJumpToPage, &QLineEdit::textChanged, this, &DialogJumpToPage::jumpTextChanged);
}

ZapFR::Client::DialogJumpToPage::~DialogJumpToPage()
{
    delete ui;
}

void ZapFR::Client::DialogJumpToPage::reset(uint64_t currentPage, uint64_t totalPageCount, std::function<void(uint64_t)> callback)
{
    mCurrentPage = currentPage;
    mTotalPageCount = totalPageCount;
    mCallback = callback;

    ui->labelTotalPageCount->setText(QString("/ %1").arg(mTotalPageCount));
    ui->lineEditJumpToPage->setText(QString::number(mCurrentPage));
    ui->lineEditJumpToPage->setFocus();
    ui->lineEditJumpToPage->selectAll();
}

uint64_t ZapFR::Client::DialogJumpToPage::pageToJumpTo() const
{
    auto text = ui->lineEditJumpToPage->text();

    uint64_t parsedValue{0};
    if (!Poco::NumberParser::tryParseUnsigned64(text.toStdString(), parsedValue) || parsedValue < 1)
    {
        return 1;
    }

    if (parsedValue > mTotalPageCount)
    {
        return mTotalPageCount;
    }

    return parsedValue;
}

void ZapFR::Client::DialogJumpToPage::jumpTextChanged(const QString& text)
{
    QAbstractButton* okButton{nullptr};
    for (const auto& button : ui->buttonBox->buttons())
    {
        if (ui->buttonBox->buttonRole(button) == QDialogButtonBox::ButtonRole::AcceptRole)
        {
            okButton = button;
            break;
        }
    }
    if (okButton == nullptr)
    {
        return;
    }

    uint64_t parsedValue{0};
    okButton->setEnabled(Poco::NumberParser::tryParseUnsigned64(text.toStdString(), parsedValue));
}

std::function<void(uint64_t)> ZapFR::Client::DialogJumpToPage::callback() const
{
    return mCallback;
}
