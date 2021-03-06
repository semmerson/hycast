/**
 * Set whose entries are also linked together into a list.
 *
 *        File: LinkedMap.cpp
 *  Created on: August 20, 2020
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

#include "config.h"

#include "LinkedSet.h"

#include "error.h"

#include <unordered_set>

namespace hycast {

/**
 * @tparam VALUE  Value type
 */
template<class VALUE>
class LinkedSet<VALUE>::Impl final
{
    using Index = unsigned short;

    /**
     * Mapped-to entry in the map.
     */
    struct Entry final
    {
        VALUE value; ///< User's value
        Index prev;  ///< Index of previous entry (towards the head)
        Index next;  ///< Index of subsequent entry (towards the tail)

        Entry(VALUE& value, Index& prev)
            : value(value)
            , prev(prev)
            , next(0)
        {}
    };

    std::unordered_map<Index, Entry> map;  ///< Map from index to entry
    Index                            head; ///< Index of head of list
    Index                            tail; ///< Index of tail of list
    Index                            nextIndex;

public:
    /**
     * Default constructs.
     */
    Impl()
        : map()
        , head(0)
        , tail(0)
        , nextIndex(0)
    {}

    /**
     * Constructs with an initial number of buckets for the set.
     *
     * @param[in] initSize  Initial size
     */
    Impl(const size_t initSize)
        : map(initSize)
        , head(0)
        , tail(0)
        , nextIndex(0)
    {}

    /**
     * Returns the number of entries.
     *
     * @return Number of entries
     */
    size_t size()
    {
        return map.size();
    }

    /**
     * Adds an entry. If a new entry is created, then it is added to the tail of
     * the list.
     *
     * @param[in] value  Value to be added
     * @return           Pointer to value in set
     */
    const VALUE* add(VALUE& value)
    {
        if (!key)
            throw INVALID_ARGUMENT("Key is invalid");

        auto pair = map.insert(std::make_pair(key, Entry(value, tail)));

        if (pair.second) {
            // New entry
            if (tail) {
                map[tail].next = key;
            }
            else {
                head = key;
            }
            tail = key;
        }

        return &pair.first->second.value;
    }

    /**
     * Returns the value that corresponds to a key.
     *
     * @param[in] key        Key
     * @retval    `nullptr`  No such value
     * @return               Pointer to value. Valid only while no changes are
     *                       made.
     */
    VALUE* find(const KEY& key)
    {
        auto iter = map.find(key);
        return (iter == map.end())
                ? nullptr
                : &iter->second.value;
    }

    /**
     * Removes an entry.
     *
     * @param[in] key           Key of entry to be removed
     * @return                  Value associated with key
     * @throws InvalidArgument  No such entry
     */
    VALUE remove(const KEY& key)
    {
        auto iter = map.find(key);

        if (iter == map.end())
            throw INVALID_ARGUMENT("No such entry");

        Entry& entry = iter->second;

        if (entry.prev) {
            map[entry.prev].next = entry.next;
        }
        else {
            head = entry.next;
        }

        if (entry.next) {
            map[entry.next].prev = entry.prev;
        }
        else {
            tail = entry.prev;
        }

        return entry.value;
    }

    /**
     * Returns the key of the head of the list.
     *
     * @return Key of head of list. Will test false if the list is empty.
     */
    KEY getHead()
    {
        return head;
    }

    /**
     * Returns the key of the tail of the list.
     *
     * @return Key of tail of list. Will test false if the list is empty.
     */
    KEY getTail()
    {
        return tail;
    }
};

template<class KEY, class VALUE>
LinkedMap<KEY,VALUE>::LinkedMap()
    : pImpl() {
}

template<class KEY, class VALUE>
LinkedMap<KEY,VALUE>::LinkedMap(const size_t initSize)
    : pImpl(new Impl(initSize)) {
}

template<class KEY, class VALUE>
size_t LinkedMap<KEY,VALUE>::size() const noexcept {
    return pImpl->size();
}

template<class KEY, class VALUE>
VALUE* LinkedMap<KEY,VALUE>::add(const KEY& key, VALUE& value) {
    return pImpl->add(key, value);
}

template<class KEY, class VALUE>
VALUE* LinkedMap<KEY,VALUE>::find(const KEY& key) {
    return pImpl->find(key);
}

template<class KEY, class VALUE>
VALUE LinkedMap<KEY,VALUE>::remove(const KEY& key) {
    return pImpl->remove(key);
}

template<class KEY, class VALUE>
KEY LinkedMap<KEY,VALUE>::getHead() {
    return pImpl->getHead();
}

template<class KEY, class VALUE>
KEY LinkedMap<KEY,VALUE>::getTail() {
    return pImpl->getTail();
}

} // namespace
