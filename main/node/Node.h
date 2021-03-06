/**
 * A node in the peer-to-peer network
 *
 *        File: Node.h
 *  Created on: Jun 3, 2020
 *      Author: Steven R. Emmerson
 *
 *    Copyright 2021 University Corporation for Atmospheric Research
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef MAIN_NODE_NODE_H_
#define MAIN_NODE_NODE_H_

#include "McastProto.h"
#include "P2pMgr.h"
#include "Repository.h"

#include <memory>

namespace hycast {

/**
 * Abstract base class for a node on a data-product distribution network.
 */
class Node
{
protected:
    class Impl;

    std::shared_ptr<Impl> pImpl;

    Node();

    Node(Impl* impl);

public:
    virtual ~Node() =0;

    operator bool() {
        return static_cast<bool>(pImpl);
    }

    /**
     * Executes this instance.
     */
    void operator()() const /*=0*/;

    /**
     * Halts execution of this instance. Causes `operator()()` to return.
     */
    void halt() const /*=0*/;
};

class Publisher final : public Node
{
    class Impl;

public:
    /**
     * Default constructs. Resulting instance will test false.
     */
    Publisher();

    /**
     * Constructs.
     *
     * @param[in] p2pInfo  Information about the local P2P server
     * @param[in] grpAddr  Address to which products will be multicast
     * @param[in] repo     Publisher's repository
     */
    Publisher(
            P2pInfo&        p2pInfo,
            const SockAddr& grpAddr,
            PubRepo&        repo);

    /**
     * Links to a file (which could be a directory) that's outside the
     * repository. All regular files will be published.
     *
     * @param[in] pathname       Absolute pathname (with no trailing '/') of the
     *                           file or directory to be linked to
     * @param[in] prodName       Product name if the pathname references a file
     *                           and Product name prefix if the pathname
     *                           references a directory
     * @throws InvalidArgument  `pathname` is empty or a relative pathname
     * @throws InvalidArgument  `prodName` is invalid
     */
    void link(
            const std::string& pathname,
            const std::string& prodName);

#if 1
    /**
     * Executes this instance.
     */
    void operator()() const;
#endif
};

class Subscriber final : public Node
{
    class Impl;

public:
    /**
     * Constructs.
     *
     * @param[in] srcMcastAddrs  Source-specific multicast addresses
     * @param[in] p2pInfo        Information about the local P2P server
     * @param[in] ServerPool     Pool of remote P2P servers
     * @param[in] repo           Subscriber's product repository
     */
    Subscriber(
            const SrcMcastAddrs& srcMcastAddrs,
            P2pInfo&             p2pInfo,
            ServerPool&          p2pSrvrPool,
            SubRepo&             repo);

#if 0
    /**
     * Executes this instance.
     */
    void operator()() const;
#endif
};

} // namespace

#endif /* MAIN_NODE_NODE_H_ */
