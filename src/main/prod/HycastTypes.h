/**
 * This file defines primitive types common to the Hycast package.
 *
 * Copyright 2016 University Corporation for Atmospheric Research. All rights
 * reserved. See the file COPYING in the top-level source-directory for
 * licensing conditions.
 *
 *   @file: HycastTypes.h
 * @author: Steven R. Emmerson
 */

#ifndef HYCASTTYPES_H_
#define HYCASTTYPES_H_

#include <cstdint>

namespace hycast {

typedef uint16_t Version;
typedef uint32_t ProdSize;
typedef uint32_t ChunkIndex;
typedef uint32_t ChunkOffset;
typedef uint16_t ChunkSize;
typedef uint16_t ProdNameSize;

static const ProdSize      prodSizeMax     = UINT32_MAX;
static const ChunkIndex    chunkIndexMax   = UINT32_MAX;
static const ChunkSize     chunkSizeMax    = UINT16_MAX;
static const ProdNameSize  prodNameSizeMax = UINT16_MAX;

}

#endif /* HYCASTTYPES_H_ */
