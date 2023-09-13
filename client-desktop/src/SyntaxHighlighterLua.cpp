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

#include "SyntaxHighlighterLua.h"

ZapFR::Client::SyntaxHighlighterLua::SyntaxHighlighterLua(QTextDocument* parent) : QSyntaxHighlighter(parent)
{
    mFormatReservedKeywords.setForeground(QColor("#569CD6"));
    mFormatNumber.setForeground(QColor("#B5CEA8"));
    mFormatComments.setForeground(QColor("#6A9955"));
    mFormatText.setForeground(QColor("#CF8730"));
    mFormatTypes.setForeground(QColor("#4EC9B0"));

    static std::array<QString, 21> reservedKeywords{R"(\band\b)", R"(\bbreak\b)",    R"(\bdo\b)",     R"(\belse\b)", R"(\belseif\b)", R"(\bend\b)",   R"(\bfalse\b)",
                                                    R"(\bfor\b)", R"(\bfunction\b)", R"(\bif\b)",     R"(\bin\b)",   R"(\blocal\b)",  R"(\bnil\b)",   R"(\bnot\b)",
                                                    R"(\bor\b)",  R"(\brepeat\b)",   R"(\breturn\b)", R"(\bthen\b)", R"(\btrue\b)",   R"(\buntil\b)", R"(\bwhile\b)"};
    static std::array<QString, 8> types{R"(CurrentPost)",
                                        R"(CurrentPost\.title)",
                                        R"(CurrentPost:markAsRead\(\))",
                                        R"(CurrentPost:markAsUnread\(\))",
                                        R"(CurrentPost:flag\(\))",
                                        R"(CurrentPost:unflag\(\))",
                                        R"(CurrentPost:assignToScriptFolder\(.*?\))",
                                        R"(CurrentPost:unassignFromScriptFolder\(.*?\))"};

    for (const auto& reservedKeyword : reservedKeywords)
    {
        mRules.emplace_back(reservedKeyword, mFormatReservedKeywords);
    }
    for (const auto& t : types)
    {
        mRules.emplace_back(t, mFormatTypes);
    }
    mRules.emplace_back(R"(\b[0-9.]+\b)", mFormatNumber);
    mRules.emplace_back(R"(\-\-.*?$)", mFormatComments);
    mRules.emplace_back(R"(['"].*?['"])", mFormatText);
}

void ZapFR::Client::SyntaxHighlighterLua::highlightBlock(const QString& text)
{
    for (const auto& rule : mRules)
    {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext())
        {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(static_cast<int32_t>(match.capturedStart()), static_cast<int32_t>(match.capturedLength()), rule.format);
        }
    }
}
