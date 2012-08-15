//* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ProtocolParser_h__
#define ProtocolParser_h__

#include "HashStore.h"
#include "nsICryptoHMAC.h"

namespace mozilla {
namespace safebrowsing {

/**
 * Some helpers for parsing the safe
 */
class ProtocolParser {
public:
  struct ForwardedUpdate {
    nsCString table;
    nsCString url;
    nsCString mac;
  };

  ProtocolParser(PRUint32 aHashKey);
  ~ProtocolParser();

  nsresult Status() const { return mUpdateStatus; }

  nsresult Init(nsICryptoHash* aHasher, bool mPerClientRandomize);

  nsresult InitHMAC(const nsACString& aClientKey,
                    const nsACString& aServerMAC);
  nsresult FinishHMAC();

  void SetCurrentTable(const nsACString& aTable);

  nsresult Begin();
  nsresult AppendStream(const nsACString& aData);

  // Forget the table updates that were created by this pass.  It
  // becomes the caller's responsibility to free them.  This is shitty.
  TableUpdate *GetTableUpdate(const nsACString& aTable);
  void ForgetTableUpdates() { mTableUpdates.Clear(); }
  nsTArray<TableUpdate*> &GetTableUpdates() { return mTableUpdates; }

  // Update information.
  const nsTArray<ForwardedUpdate> &Forwards() const { return mForwards; }
  int32 UpdateWait() { return mUpdateWait; }
  bool ResetRequested() { return mResetRequested; }
  bool RekeyRequested() { return mRekeyRequested; }

private:
  nsresult ProcessControl(bool* aDone);
  nsresult ProcessMAC(const nsCString& aLine);
  nsresult ProcessExpirations(const nsCString& aLine);
  nsresult ProcessChunkControl(const nsCString& aLine);
  nsresult ProcessForward(const nsCString& aLine);
  nsresult AddForward(const nsACString& aUrl, const nsACString& aMac);
  nsresult ProcessChunk(bool* done);
  nsresult ProcessPlaintextChunk(const nsACString& aChunk);
  nsresult ProcessShaChunk(const nsACString& aChunk);
  nsresult ProcessHostAdd(const Prefix& aDomain, PRUint8 aNumEntries,
                          const nsACString& aChunk, PRUint32* aStart);
  nsresult ProcessHostSub(const Prefix& aDomain, PRUint8 aNumEntries,
                          const nsACString& aChunk, PRUint32* aStart);
  nsresult ProcessHostAddComplete(PRUint8 aNumEntries, const nsACString& aChunk,
                                  PRUint32 *aStart);
  nsresult ProcessHostSubComplete(PRUint8 numEntries, const nsACString& aChunk,
                                  PRUint32* start);
  bool NextLine(nsACString& aLine);

  void CleanupUpdates();

  enum ParserState {
    PROTOCOL_STATE_CONTROL,
    PROTOCOL_STATE_CHUNK
  };
  ParserState mState;

  enum ChunkType {
    CHUNK_ADD,
    CHUNK_SUB
  };

  struct ChunkState {
    ChunkType type;
    uint32 num;
    uint32 hashSize;
    uint32 length;
    void Clear() { num = 0; hashSize = 0; length = 0; }
  };
  ChunkState mChunkState;

  PRUint32 mHashKey;
  bool mPerClientRandomize;
  nsCOMPtr<nsICryptoHash> mCryptoHash;

  nsresult mUpdateStatus;
  nsCString mPending;

  nsCOMPtr<nsICryptoHMAC> mHMAC;
  nsCString mServerMAC;

  uint32 mUpdateWait;
  bool mResetRequested;
  bool mRekeyRequested;

  nsTArray<ForwardedUpdate> mForwards;
  nsTArray<TableUpdate*> mTableUpdates;
  TableUpdate *mTableUpdate;
};

}
}

#endif
