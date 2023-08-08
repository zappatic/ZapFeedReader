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

#ifndef ZAPFR_ENGINE_POSTLOCAL_H
#define ZAPFR_ENGINE_POSTLOCAL_H

#include "Post.h"

namespace ZapFR
{
    namespace Engine
    {
        class PostLocal : public Post
        {
          public:
            explicit PostLocal(uint64_t id);
            ~PostLocal() = default;

            void markFlagged(FlagColor flagColor) override;
            void markUnflagged(FlagColor flagColor) override;
        };
    } // namespace Engine
} // namespace ZapFR

#endif // ZAPFR_ENGINE_POSTLOCAL_H