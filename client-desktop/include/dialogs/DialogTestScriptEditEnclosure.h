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

#ifndef ZAPFR_CLIENT_DIALOGTESTSCRIPTEDITENCLOSURE_H
#define ZAPFR_CLIENT_DIALOGTESTSCRIPTEDITENCLOSURE_H

#include <QDialog>

#include "../ClientGlobal.h"

namespace Ui
{
    class DialogTestScriptEditEnclosure;
}

namespace ZapFR
{
    namespace Client
    {
        class DialogTestScriptEditEnclosure : public QDialog
        {
            Q_OBJECT

          public:
            explicit DialogTestScriptEditEnclosure(QWidget* parent = nullptr);
            ~DialogTestScriptEditEnclosure();
            DialogTestScriptEditEnclosure(const DialogTestScriptEditEnclosure& e) = delete;
            DialogTestScriptEditEnclosure& operator=(const DialogTestScriptEditEnclosure&) = delete;
            DialogTestScriptEditEnclosure(DialogTestScriptEditEnclosure&&) = delete;
            DialogTestScriptEditEnclosure& operator=(DialogTestScriptEditEnclosure&&) = delete;

            enum class DisplayMode
            {
                Add,
                Edit
            };

            DisplayMode displayMode() const noexcept { return mDisplayMode; }
            void reset(DisplayMode dm, const QString& url, const QString& mimeType, uint64_t size);
            QString url() const noexcept;
            QString mimeType() const noexcept;
            uint64_t size() const;

          private:
            Ui::DialogTestScriptEditEnclosure* ui;
            DisplayMode mDisplayMode{DisplayMode::Add};
        };
    } // namespace Client
} // namespace ZapFR

#endif // ZAPFR_CLIENT_DialogTestScriptEditEnclosure_H
