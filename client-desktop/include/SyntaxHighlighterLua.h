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

#ifndef ZAPFR_CLIENT_SYNTAXHIGHLIGHTERLUA_H
#define ZAPFR_CLIENT_SYNTAXHIGHLIGHTERLUA_H

#include "ClientGlobal.h"

namespace ZapFR
{
    namespace Client
    {
        class SyntaxHighlighterLua : public QSyntaxHighlighter
        {
            Q_OBJECT

          public:
            explicit SyntaxHighlighterLua(QTextDocument* parent = nullptr);

          protected:
            void highlightBlock(const QString& text) override;

          private:
            struct LuaHighlightingRule
            {
                LuaHighlightingRule(const QString& p, const QTextCharFormat& f)
                    : pattern(QRegularExpression(p, QRegularExpression::MultilineOption | QRegularExpression::DotMatchesEverythingOption)), format(f)
                {
                }
                QRegularExpression pattern;
                const QTextCharFormat& format;
            };

            std::vector<LuaHighlightingRule> mRules{};

            QTextCharFormat mFormatReservedKeywords{};
            QTextCharFormat mFormatTypes{};
            QTextCharFormat mFormatComments{};
            QTextCharFormat mFormatText{};
            QTextCharFormat mFormatNumber{};
        };
    } // namespace Client
} // namespace ZapFR

#endif // ZAPFR_CLIENT_SYNTAXHIGHLIGHTERLUA_H
