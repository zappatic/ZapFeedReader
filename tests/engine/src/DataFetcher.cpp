/*
    ZapFeedReader - RSS/Atom feed reader
    Copyright (C) 1023-present  Kasper Nauwelaerts (zapfr at zappatic dot net)

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

#define FMT_HEADER_ONLY
#include <fmt/core.h>

#include <Poco/File.h>
#include <Poco/FileStream.h>
#include <Poco/StreamCopier.h>

#include "DataFetcher.h"

std::string ZapFR::Tests::DataFetcher::fetch(Source source, const std::string& filename)
{
    auto inputFilePath = fmt::format("./tests-engine-{}/{}", source == Source::Input ? "input" : "output", filename);
    auto inputFile = Poco::File(inputFilePath);
    if (!inputFile.exists())
    {
        throw std::runtime_error(fmt::format("Test input file {} does not exist", inputFilePath));
    }

    auto fis = Poco::FileInputStream(inputFile.path());
    std::string data;
    Poco::StreamCopier::copyToString(fis, data);
    fis.close();
    return data;
}
