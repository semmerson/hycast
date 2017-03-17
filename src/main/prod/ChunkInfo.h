/**
 * This file declares metadata about a chunk of data.
 *
 * Copyright 2016 University Corporation for Atmospheric Research. All rights
 * reserved. See the file COPYING in the top-level source-directory for
 * licensing conditions.
 *
 *   @file: ChunkInfo.h
 * @author: Steven R. Emmerson
 */

#ifndef CHUNKINFO_H_
#define CHUNKINFO_H_

#include "HycastTypes.h"
#include "ProdIndex.h"
#include "Serializable.h"

#include <cstddef>
#include <functional>

namespace hycast {

class ChunkInfo final : public Serializable<ChunkInfo> {
    ProdIndex   prodIndex;
    ChunkIndex  chunkIndex;
    ChunkSize   chunkSize;

public:
    /**
     * Constructs.
     * @param[in] prodIndex   Product index
     * @param[in] chunkIndex  Chunk index
     * @param[in] chunkSize   Chunk size in bytes
     */
    ChunkInfo(
            const ProdIndex  prodIndex,
            const ChunkIndex chunkIndex,
            const ChunkSize  chunkSize)
        : prodIndex(prodIndex)
        , chunkIndex(chunkIndex)
        , chunkSize(chunkSize)
    {}

    /**
     * Default constructs.
     */
    ChunkInfo()
        : ChunkInfo(0, 0, 0)
    {}

    ChunkInfo(
            Decoder& decoder,
            unsigned version);

    /**
     * Returns the product index.
     * @return the product index
     */
    ProdIndex getProdIndex() const {return prodIndex;}

    /**
     * Returns the chunk index.
     * @return the chunk index
     */
    ChunkIndex getIndex() const {return chunkIndex;}

    /**
     * Returns the chunk size in bytes.
     * @return the chunk size in bytes
     */
    ChunkIndex getSize() const {return chunkSize;}

    /**
     * Returns the size of a serialized instance in bytes.
     * @param[in] version  Protocol version
     * @return the size of a serialized instance in bytes
     */
    static size_t getStaticSerialSize(const unsigned version) noexcept
    {
        return ProdIndex::getStaticSerialSize(version) +
                Codec::getSerialSize(sizeof(ChunkIndex)) +
                Codec::getSerialSize(sizeof(ChunkSize));
    }

    /**
     * Returns the size, in bytes, of the serialized representation of this
     * instance.
     * @param[in] version  Protocol version
     * @return the serialized size, in bytes, of this instance
     */
    size_t getSerialSize(unsigned version) const noexcept
    {
        return getStaticSerialSize(version);
    }

    /**
     * Indicates if this instance equals another.
     * @param[in] that  Other instance
     * @retval `true` iff this instance equals the other
     * @exceptionsafety Nothrow
     * @threadsafety    Safe
     */
    bool operator==(const ChunkInfo& that) const noexcept;

    /**
     * Returns the hash code of this instance.
     * @return This instance's hash code
     * @execeptionsafety Nothrow
     * @threadsafety     Safe
     */
    size_t hash() const noexcept {
        return std::hash<ChunkIndex>()(chunkIndex) | prodIndex.hash();
    }

    /**
     * Indicates if this instance is less than another.
     * @param[in] that  Other instance
     * @return `true` iff this instance is less than the other
     * @exceptionsafety Nothrow
     * @threadsafety    Safe
     */
    bool operator<(const ChunkInfo& that) const noexcept {
        return prodIndex < that.prodIndex ||
                (prodIndex == that.prodIndex && chunkIndex < that.chunkIndex);
    }

    /**
     * Serializes this instance to an encoder.
     * @param[out] encoder  Encoder
     * @param[in]  version  Protocol version
     * @return Number of bytes written
     */
    size_t serialize(
            Encoder&       encoder,
            const unsigned version) const;

    /**
     * Returns a new instance corresponding to a serialized representation in a
     * decoder.
     * @param[in]  decoder  Decoder
     * @param[in]  version  Protocol version
     * @exceptionsafety Basic
     * @threadsafety    Compatible but not thread-safe
     */
    static ChunkInfo deserialize(
            Decoder&          decoder,
            const unsigned    version);
};

} // namespace

#include <functional>

namespace std {
    template<> struct hash<hycast::ChunkInfo> {
        size_t operator()(const hycast::ChunkInfo& info) const noexcept {
            return info.hash();
        }
    };

    template<> struct less<hycast::ChunkInfo> {
        bool operator()(const hycast::ChunkInfo& info1,
                const hycast::ChunkInfo& info2) const noexcept {
            return info1 < info2;
        }
    };

    template<> struct equal_to<hycast::ChunkInfo> {
        bool operator()(const hycast::ChunkInfo& info1,
                const hycast::ChunkInfo& info2) const noexcept {
            return info1 == info2;
        }
    };
}

#endif /* CHUNKINFO_H_ */
