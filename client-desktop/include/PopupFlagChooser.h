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

#ifndef ZAPFR_CLIENT_POPUPFLAGCHOOSER_H
#define ZAPFR_CLIENT_POPUPFLAGCHOOSER_H

#include "ClientGlobal.h"
#include "Flag.h"
#include "Utilities.h"

namespace Ui
{
    class PopupFlagChooser;
}

namespace ZapFR
{
    namespace Client
    {
        class PopupFlag : public QWidget
        {
            Q_OBJECT
          public:
            explicit PopupFlag(QWidget* parent = nullptr);
            ~PopupFlag() = default;

            void setFlagColor(ZapFR::Engine::FlagColor flagColor);
            ZapFR::Engine::FlagColor flagColor() const noexcept;
            void setFlagStyle(Utilities::FlagStyle flagStyle);

          signals:
            void flagClicked(ZapFR::Engine::FlagColor flagColor, Utilities::FlagStyle flagStyle);

          protected:
            void paintEvent(QPaintEvent* event) override;
            void mouseReleaseEvent(QMouseEvent* event) override;

          private:
            ZapFR::Engine::FlagColor mFlagColor;
            Utilities::FlagStyle mFlagStyle{Utilities::FlagStyle::Unfilled};
        };

        class PopupFlagChooser : public QWidget
        {
            Q_OBJECT

          public:
            explicit PopupFlagChooser(QWidget* parent = nullptr);
            ~PopupFlagChooser();

            void showWithSelectedColors(const QList<QVariant> flagColors);

          signals:
            void flagToggled(ZapFR::Engine::FlagColor flagColor, Utilities::FlagStyle flagStyle);

          protected:
            void focusOutEvent(QFocusEvent* event) override;

          private:
            Ui::PopupFlagChooser* ui;
            std::vector<PopupFlag*> mFlags{};
        };
    } // namespace Client
} // namespace ZapFR

#endif // POPUPFLAGCHOOSER_H
