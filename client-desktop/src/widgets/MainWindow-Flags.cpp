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

#include "./ui_MainWindow.h"
#include "ZapFR/Agent.h"
#include "widgets/MainWindow.h"

void ZapFR::Client::MainWindow::reloadUsedFlagColors(bool forceReload)
{
    auto index = ui->treeViewSources->currentIndex();
    if (index.isValid())
    {
        auto sourceID = index.data(SourceTreeEntryParentSourceIDRole).toULongLong();
        if (forceReload || sourceID != mPreviouslySelectedSourceID)
        {
            ZapFR::Engine::Agent::getInstance()->queueGetUsedFlagColors(sourceID,
                                                                        [&](uint64_t affectedSourceID, const std::unordered_set<ZapFR::Engine::FlagColor>& flagColors)
                                                                        {
                                                                            QMetaObject::invokeMethod(this, [=, this]() { populateUsedFlags(affectedSourceID, flagColors); });
                                                                            mPreviouslySelectedSourceID = affectedSourceID;
                                                                        });
        }
    }
}

void ZapFR::Client::MainWindow::populateUsedFlags(uint64_t /*sourceID*/, const std::unordered_set<ZapFR::Engine::FlagColor>& flagColors)
{
    ui->widgetFilterFlagBlue->setHidden(!flagColors.contains(ZapFR::Engine::FlagColor::Blue));
    ui->widgetFilterFlagGreen->setHidden(!flagColors.contains(ZapFR::Engine::FlagColor::Green));
    ui->widgetFilterFlagYellow->setHidden(!flagColors.contains(ZapFR::Engine::FlagColor::Yellow));
    ui->widgetFilterFlagOrange->setHidden(!flagColors.contains(ZapFR::Engine::FlagColor::Orange));
    ui->widgetFilterFlagRed->setHidden(!flagColors.contains(ZapFR::Engine::FlagColor::Red));
    ui->widgetFilterFlagPurple->setHidden(!flagColors.contains(ZapFR::Engine::FlagColor::Purple));

    if (!flagColors.contains(mFlagFilter))
    {
        mFlagFilter = ZapFR::Engine::FlagColor::Gray;
    }
}

void ZapFR::Client::MainWindow::connectFlagStuff()
{
    ui->widgetFilterFlagBlue->setFlagColor(ZapFR::Engine::FlagColor::Blue);
    ui->widgetFilterFlagGreen->setFlagColor(ZapFR::Engine::FlagColor::Green);
    ui->widgetFilterFlagYellow->setFlagColor(ZapFR::Engine::FlagColor::Yellow);
    ui->widgetFilterFlagOrange->setFlagColor(ZapFR::Engine::FlagColor::Orange);
    ui->widgetFilterFlagRed->setFlagColor(ZapFR::Engine::FlagColor::Red);
    ui->widgetFilterFlagPurple->setFlagColor(ZapFR::Engine::FlagColor::Purple);

    static std::vector<PopupFlag*> flags{ui->widgetFilterFlagBlue,   ui->widgetFilterFlagGreen, ui->widgetFilterFlagYellow,
                                         ui->widgetFilterFlagOrange, ui->widgetFilterFlagRed,   ui->widgetFilterFlagPurple};
    for (const auto& flag : flags)
    {
        flag->setFlagStyle(Utilities::FlagStyle::Unfilled);
        connect(flag, &PopupFlag::flagClicked,
                [&](PopupFlag* clickedFlag)
                {
                    for (const auto& f : flags)
                    {
                        if (f != clickedFlag)
                        {
                            f->setFlagStyle(Utilities::FlagStyle::Unfilled);
                        }
                    }

                    switch (clickedFlag->flagStyle())
                    {
                        case Utilities::FlagStyle::Filled:
                        {
                            clickedFlag->setFlagStyle(Utilities::FlagStyle::Unfilled);
                            mFlagFilter = ZapFR::Engine::FlagColor::Gray;
                            break;
                        }
                        case Utilities::FlagStyle::Unfilled:
                        {
                            clickedFlag->setFlagStyle(Utilities::FlagStyle::Filled);
                            mFlagFilter = clickedFlag->flagColor();
                            break;
                        }
                    }
                    mCurrentPostPage = 1;
                    reloadPosts();
                    updateActivePostFilter();
                });
    }
}
