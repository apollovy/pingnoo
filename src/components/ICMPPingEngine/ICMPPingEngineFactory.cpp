/*
 * Copyright (C) 2020 Adrian Carpenter
 *
 * This file is part of Pingnoo (https://github.com/nedrysoft/pingnoo)
 *
 * An open-source cross-platform traceroute analyser.
 *
 * Created by Adrian Carpenter on 27/03/2020.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "ICMPPingEngineFactory.h"
#include "ICMPPingEngine.h"
#include "ICMPPingReceiverWorker.h"

/**
 * @brief       Private class to store the ping engines instance data.
 */
class Nedrysoft::ICMPPingEngine::ICMPPingEngineFactoryData {
    public:
        /**
         * @brief       Constructs a ICMPPingEngineFactoryData.
         *
         * @param[in]   parent the ICMPPingEngineFactory instance that this data belongs to.
         */
        ICMPPingEngineFactoryData(Nedrysoft::ICMPPingEngine::ICMPPingEngineFactory *parent) :
                m_factory(parent) {

        }

        friend class ICMPPingEngineFactory;

    private:
        Nedrysoft::ICMPPingEngine::ICMPPingEngineFactory *m_factory;

        QList<Nedrysoft::ICMPPingEngine::ICMPPingEngine *> m_engineList;
};

Nedrysoft::ICMPPingEngine::ICMPPingEngineFactory::ICMPPingEngineFactory() :
        d(std::make_shared<Nedrysoft::ICMPPingEngine::ICMPPingEngineFactoryData>(this)) {

}

Nedrysoft::ICMPPingEngine::ICMPPingEngineFactory::~ICMPPingEngineFactory() {
    qDeleteAll(d->m_engineList);

    auto receiverWorker = Nedrysoft::ICMPPingEngine::ICMPPingReceiverWorker::getInstance(true);

    if (receiverWorker) {
        delete receiverWorker;
    }

    d.reset();
}

auto Nedrysoft::ICMPPingEngine::ICMPPingEngineFactory::createEngine(
        Nedrysoft::Core::IPVersion version ) -> Nedrysoft::RouteAnalyser::IPingEngine * {

    auto engineInstance = new Nedrysoft::ICMPPingEngine::ICMPPingEngine(version);

    d->m_engineList.append(engineInstance);

    return engineInstance;
}

auto Nedrysoft::ICMPPingEngine::ICMPPingEngineFactory::saveConfiguration() -> QJsonObject {
    return QJsonObject();
}

auto Nedrysoft::ICMPPingEngine::ICMPPingEngineFactory::loadConfiguration(QJsonObject configuration) -> bool {
    Q_UNUSED(configuration)

    return false;
}

auto Nedrysoft::ICMPPingEngine::ICMPPingEngineFactory::description() -> QString {
    return tr("ICMP Socket");
}

auto Nedrysoft::ICMPPingEngine::ICMPPingEngineFactory::priority() -> double {
#if defined(Q_OS_LINUX)
    auto socket = Nedrysoft::ICMPSocket::ICMPSocket::createReadSocket(Nedrysoft::ICMPSocket::V4);

    if (socket) {
        delete socket;

        return 1;
    }

    return 0;
#endif
    return 1;
}

auto Nedrysoft::ICMPPingEngine::ICMPPingEngineFactory::available() -> bool {
#if defined(Q_OS_LINUX)
    auto socket = Nedrysoft::ICMPSocket::ICMPSocket::createReadSocket(Nedrysoft::ICMPSocket::V4);

    if (socket) {
        delete socket;

        return true;
    }

    return false;
#endif
    return true;
}

auto Nedrysoft::ICMPPingEngine::ICMPPingEngineFactory::deleteEngine(
        Nedrysoft::RouteAnalyser::IPingEngine *engine) -> bool {

    auto pingEngine = qobject_cast<Nedrysoft::ICMPPingEngine::ICMPPingEngine *>(engine);

    if (d->m_engineList.contains(pingEngine)) {
        engine->stop();
        d->m_engineList.removeAll(pingEngine);
        pingEngine->deleteLater();
    }

    return true;
}

