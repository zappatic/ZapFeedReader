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

#include "widgets/FrameFlagFilters.h"
#include "./ui_MainWindow.h"
#include "ZapFR/Agent.h"
#include "widgets/MainWindow.h"
#include "widgets/TreeViewSources.h"

ZapFR::Client::FrameFlagFilters::FrameFlagFilters(QWidget* parent) : QFrame(parent)
{
}

void ZapFR::Client::FrameFlagFilters::setMainWindow(MainWindow* mw) noexcept
{
    mMainWindow = mw;

    auto ui = mMainWindow->getUI();
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
                    auto localUI = mMainWindow->getUI();
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
                            localUI->tableViewPosts->setFlagFilter(ZapFR::Engine::FlagColor::Gray);
                            break;
                        }
                        case Utilities::FlagStyle::Unfilled:
                        {
                            clickedFlag->setFlagStyle(Utilities::FlagStyle::Filled);
                            localUI->tableViewPosts->setFlagFilter(clickedFlag->flagColor());
                            break;
                        }
                    }
                    localUI->tableViewPosts->setPage(1);
                    localUI->tableViewPosts->reload();
                    localUI->tableViewPosts->updateActivePostFilter();
                });
    }
}

void ZapFR::Client::FrameFlagFilters::reload(bool forceReload)
{
    auto index = mMainWindow->treeViewSources()->currentIndex();
    if (index.isValid())
    {
        auto sourceID = index.data(TreeViewSources::Role::ParentSourceID).toULongLong();
        if (forceReload || sourceID != mMainWindow->treeViewSources()->previouslySelectedSourceID())
        {
            ZapFR::Engine::Agent::getInstance()->queueGetUsedFlagColors(sourceID,
                                                                        [&](uint64_t affectedSourceID, const std::unordered_set<ZapFR::Engine::FlagColor>& flagColors)
                                                                        {
                                                                            QMetaObject::invokeMethod(this, [=, this]() { populateUsedFlags(affectedSourceID, flagColors); });
                                                                            mMainWindow->treeViewSources()->setPreviouslySelectedSourceID(affectedSourceID);
                                                                        });
        }
    }
}

void ZapFR::Client::FrameFlagFilters::populateUsedFlags(uint64_t /*sourceID*/, const std::unordered_set<ZapFR::Engine::FlagColor>& flagColors)
{
    auto ui = mMainWindow->getUI();
    ui->widgetFilterFlagBlue->setHidden(!flagColors.contains(ZapFR::Engine::FlagColor::Blue));
    ui->widgetFilterFlagGreen->setHidden(!flagColors.contains(ZapFR::Engine::FlagColor::Green));
    ui->widgetFilterFlagYellow->setHidden(!flagColors.contains(ZapFR::Engine::FlagColor::Yellow));
    ui->widgetFilterFlagOrange->setHidden(!flagColors.contains(ZapFR::Engine::FlagColor::Orange));
    ui->widgetFilterFlagRed->setHidden(!flagColors.contains(ZapFR::Engine::FlagColor::Red));
    ui->widgetFilterFlagPurple->setHidden(!flagColors.contains(ZapFR::Engine::FlagColor::Purple));

    if (!flagColors.contains(ui->tableViewPosts->flagFilter()))
    {
        ui->tableViewPosts->setFlagFilter(ZapFR::Engine::FlagColor::Gray);
    }
}
