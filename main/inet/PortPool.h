/**
 * A thread-safe queue of port-numbers.
 *
 *        File: PortPool.h
 *  Created on: Jul 13, 2019
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

#ifndef MAIN_PEER_PORTPOOL_H_
#define MAIN_PEER_PORTPOOL_H_

#include <memory>
#include <netinet/in.h>

namespace hycast {

class PortPool
{
    class                 Impl;
    std::shared_ptr<Impl> pImpl;

public:
    /**
     * Default constructs. Will test false.
     */
    PortPool();

    /**
     * Constructs from a range of port-numbers.
     *
     * @param[in] min  Minimum port number (inclusive) in host byte-order
     * @param[in] num  Number of port numbers
     */
    PortPool(
            in_port_t min,
            unsigned  num);

    in_port_t getMin() const noexcept;

    unsigned getNum() const noexcept;

    /**
     * Indicates if this instance is valid (i.e., not default constructed).
     *
     * @retval `true`   Valid
     * @retval `false`  Not valid
     */
    operator bool() const noexcept;

    /**
     * Returns the number of port-numbers in the queue.
     *
     * @return        Number of port numbers in the queue
     * @threadsafety  Safe
     */
    int size() const;

    /**
     * Removes port-number at the head of the queue. Blocks until one is
     * available.
     *
     * @return       Port number at head of queue in host byte-order
     * @threadsafety Safe
     */
    in_port_t take() const;

    /**
     * Adds a port-number to the end of the queue.
     *
     * @param[in] port  The port-number in host byte-order
     * @threadsafety    Safe
     */
    void add(in_port_t port) const;
};

} // namespace

#endif /* MAIN_PEER_PORTPOOL_H_ */
