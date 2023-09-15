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

#include "ZapFR/AgentRunnable.h"
#include "ZapFR/Agent.h"
#include "ZapFR/base/Source.h"

ZapFR::Engine::AgentRunnable::AgentRunnable(uint64_t sourceID) : mSourceID(sourceID)
{
}

void ZapFR::Engine::AgentRunnable::run()
{
    static const auto logAndDemangleException = [](const std::optional<std::unique_ptr<Source>>& source, const std::string& message, const char* className)
    {
        if (source.has_value())
        {
            int32_t status;
            char* demangled = abi::__cxa_demangle(className, 0, 0, &status);
            auto errorMessage = fmt::format("{} ({})", message, demangled);

            source.value()->updateLastError(errorMessage);
            ZapFR::Engine::Agent::getInstance()->broadcastError(source.value()->id(), errorMessage);
            free(demangled);
        }
    };

    auto source = Source::getSource(mSourceID);
    if (source.has_value())
    {
        try
        {
            source.value()->updateLastError("");
            payload(source.value().get());
        }
        catch (const Poco::Exception& e)
        {
            logAndDemangleException(source, e.displayText(), typeid(*this).name());
            onPayloadException(source.value().get());
        }
        catch (const std::exception& e)
        {
            logAndDemangleException(source, e.what(), typeid(*this).name());
            onPayloadException(source.value().get());
        }
        catch (...)
        {
            logAndDemangleException(source, "Unknown error occurred", typeid(*this).name());
            onPayloadException(source.value().get());
        }
    }

    mIsDone = true;
}
