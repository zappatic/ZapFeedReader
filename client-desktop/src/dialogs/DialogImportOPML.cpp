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

#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QTextStream>

#include "ZapFR/OPMLParser.h"
#include "dialogs/DialogImportOPML.h"
#include "ui_DialogImportOPML.h"

ZapFR::Client::DialogImportOPML::DialogImportOPML(QWidget* parent) : DialogWithSourcesAndFolders(parent), ui(new Ui::DialogImportOPML)
{
    ui->setupUi(this);
    setComboBoxSources(ui->comboBoxSources);
    setComboBoxFolders(ui->comboBoxFolder);
    connect(ui->pushButtonChooseOPMLFile, &QPushButton::clicked, this, &DialogImportOPML::chooseOPMLFile);

    for (const auto& button : ui->buttonBox->buttons())
    {
        if (ui->buttonBox->buttonRole(button) == QDialogButtonBox::ButtonRole::AcceptRole)
        {
            button->setText(tr("Import"));
            break;
        }
    }
}

ZapFR::Client::DialogImportOPML::~DialogImportOPML()
{
    delete ui;
}

void ZapFR::Client::DialogImportOPML::chooseOPMLFile(bool /*checked*/)
{
    auto chosenPath = QFileDialog::getOpenFileName(this, tr("Choose an OPML file"), QString(), tr("OPML files (*.opml);;XML files (*.xml);;All files (*.*)"));
    if (!chosenPath.isNull())
    {
        auto info = QFileInfo(chosenPath);
        ui->lineEditChosenOPMLFile->setText(info.fileName());

        try
        {
            auto opmlFile = QFile(chosenPath);
            opmlFile.open(QIODeviceBase::ReadOnly);
            mOPML = QTextStream(&opmlFile).readAll().toStdString();
            ZapFR::Engine::OPMLParser::parse(mOPML);
            // TODO: check if zero feeds found!
        }
        catch (const Poco::Exception& e)
        {
            QMessageBox mb(this);
            mb.setText(tr("Error parsing OPML file"));
            mb.setInformativeText(QString::fromStdString(e.displayText()));
            mb.setIcon(QMessageBox::Icon::Critical);
            mb.exec();
        }
    }
}

void ZapFR::Client::DialogImportOPML::reset(uint64_t selectedSourceID, uint64_t selectedFolderID)
{
    ui->lineEditChosenOPMLFile->setText("");
    mOPML.clear();
    setPreselectedSourceAndFolderIDs(selectedSourceID, selectedFolderID);
}

std::string ZapFR::Client::DialogImportOPML::OPML() const noexcept
{
    return mOPML;
}
