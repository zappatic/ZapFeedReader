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

#ifndef ZAPFR_CLIENT_DIALOGPREFERENCES_H
#define ZAPFR_CLIENT_DIALOGPREFERENCES_H

#include "../ClientGlobal.h"
#include "widgets/MainWindow.h"

namespace Ui
{
    class DialogPreferences;
}

namespace ZapFR
{
    namespace Client
    {
        class DialogPreferences : public QDialog
        {
            Q_OBJECT

          public:
            explicit DialogPreferences(QWidget* parent = nullptr);
            ~DialogPreferences();
            DialogPreferences(const DialogPreferences& e) = delete;
            DialogPreferences& operator=(const DialogPreferences&) = delete;
            DialogPreferences(DialogPreferences&&) = delete;
            DialogPreferences& operator=(DialogPreferences&&) = delete;

            void reset();
            Theme theme() const;
            uint16_t uiFontSize() const;
            uint16_t postFontSize() const;
            RefreshBehaviour refreshBehaviour() const;
            uint64_t autoRefreshInterval() const;
            bool autoRefreshEnabled() const;
            bool detectBrowsersEnabled() const;
            bool hideLocalSource() const;

          private:
            Ui::DialogPreferences* ui;
        };
    } // namespace Client
} // namespace ZapFR

#endif // ZAPFR_CLIENT_DIALOGPREFERENCES_H
