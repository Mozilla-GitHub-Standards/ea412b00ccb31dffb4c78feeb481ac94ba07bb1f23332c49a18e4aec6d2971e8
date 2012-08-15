//* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "ChunkSet.h"

namespace mozilla {
namespace safebrowsing {

nsresult
ChunkSet::Serialize(nsACString& aChunkStr)
{
  aChunkStr.Truncate();

  PRUint32 i = 0;
  while (i < mChunks.Length()) {
    if (i != 0) {
      aChunkStr.Append(',');
    }
    aChunkStr.AppendInt((PRInt32)mChunks[i]);

    PRUint32 first = i;
    PRUint32 last = first;
    i++;
    while (i < mChunks.Length() && (mChunks[i] == mChunks[i - 1] + 1 || mChunks[i] == mChunks[i - 1])) {
      last = i++;
    }

    if (last != first) {
      aChunkStr.Append('-');
      aChunkStr.AppendInt((PRInt32)mChunks[last]);
    }
  }

  return NS_OK;
}

nsresult
ChunkSet::Set(PRUint32 aChunk)
{
  PRUint32 idx = mChunks.BinaryIndexOf(aChunk);
  if (idx == nsTArray<uint32>::NoIndex) {
    mChunks.InsertElementSorted(aChunk);
  }
  return NS_OK;
}

nsresult
ChunkSet::Unset(PRUint32 aChunk)
{
  mChunks.RemoveElementSorted(aChunk);

  return NS_OK;
}

bool
ChunkSet::Has(PRUint32 aChunk) const
{
  return mChunks.BinaryIndexOf(aChunk) != nsTArray<uint32>::NoIndex;
}

nsresult
ChunkSet::Merge(const ChunkSet& aOther)
{
  const uint32 *dupIter = aOther.mChunks.Elements();
  const uint32 *end = aOther.mChunks.Elements() + aOther.mChunks.Length();

  for (const uint32 *iter = dupIter; iter != end; iter++) {
    nsresult rv = Set(*iter);
    NS_ENSURE_SUCCESS(rv, rv);
  }

  return NS_OK;
}

nsresult
ChunkSet::Remove(const ChunkSet& aOther)
{
  uint32 *addIter = mChunks.Elements();
  uint32 *end = mChunks.Elements() + mChunks.Length();

  for (uint32 *iter = addIter; iter != end; iter++) {
    if (!aOther.Has(*iter)) {
      *addIter = *iter;
      addIter++;
    }
  }

  mChunks.SetLength(addIter - mChunks.Elements());

  return NS_OK;
}

void
ChunkSet::Clear()
{
  mChunks.Clear();
}

}
}
