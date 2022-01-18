/*
 * FakeTLog.actor.h
 *
 * This source file is part of the FoundationDB open source project
 *
 * Copyright 2013-2021 Apple Inc. and the FoundationDB project authors
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

#if defined(NO_INTELLISENSE) && !defined(FDBSERVER_PTXN_TEST_FAKETLOG_ACTOR_G_H)
#define FDBSERVER_PTXN_TEST_FAKETLOG_ACTOR_G_H
#include "fdbserver/ptxn/test/FakeTLog.actor.g.h"
#elif !defined(FDBSERVER_PTXN_TEST_FAKETLOG_ACTOR_H)
#define FDBSERVER_PTXN_TEST_FAKETLOG_ACTOR_H

#include <limits>
#include <memory>
#include <unordered_map>

#include "fdbclient/FDBTypes.h"
#include "fdbserver/ptxn/MessageSerializer.h"
#include "fdbserver/ptxn/MessageTypes.h"
#include "fdbserver/ptxn/StorageServerInterface.h"
#include "fdbserver/ptxn/test/Delay.h"
#include "fdbserver/ptxn/TLogInterface.h"
#include "flow/flow.h"

#include "flow/actorcompiler.h" // has to be last include

#pragma once

namespace ptxn::test {

struct TestDriverContext;

struct FakeTLogContext {
	// FIXME FakeTLogContext should *not* depend on TestDriverContext, should use TestEnvironment
	std::shared_ptr<TestDriverContext> pTestDriverContext;
	std::shared_ptr<TLogInterfaceBase> pTLogInterface;

	// Maximum bytes a peek would return
	// The actual bytes might be different since
	size_t maxBytesPerPeek = 1024 * 1024;

	// Maximum versions a peek would return
	int maxVersionsPerPeek = std::numeric_limits<int>::max();

	// Team IDs
	std::vector<StorageTeamID> storageTeamIDs{};

	// Stores the unique versions the mutations have been used, in ascending order
	std::vector<Version> versions{};

	// Arena used for storing messages. NOTE The arena in the TestDriverContext is not used, as FakeTLogContext would
	// keep its own version of data, to simulate the "persistence" behavior.
	Arena persistenceArena{};

	using StorageTeamMessages = std::unordered_map<StorageTeamID, VectorRef<VersionSubsequenceMessage>>;
	// Stores the messages per commit version, in format
	// Commit version -- ( Storage Team ID -- (Storage Team Version, Subsequence, Message) )
	//                   ( Storage Team ID -- (Storage Team Version, Subsequence, Message) )
	//                   ...
	// Commit version -- ( Storage Team ID -- (Storage Team Version, Subsequence, Message) )
	//                   ...
	std::map<Version, StorageTeamMessages> epochVersionMessages{};

	// Stores the version range per epoch
	std::vector<std::pair<Version, Version>> epochVersionRange{};

	// Delay to simulate network latency
	RandomDelay latency = RandomDelay(0.01, 0.02);
};

ACTOR Future<Void> fakeTLog_ActivelyPush(std::shared_ptr<FakeTLogContext> pFakeTLogContext);
ACTOR Future<Void> fakeTLog_PassivelyProvide(std::shared_ptr<FakeTLogContext> pFakeTLogContext);

Future<Void> getFakeTLogActor(const MessageTransferModel model, std::shared_ptr<FakeTLogContext> pFakeTLogContext);

} // namespace ptxn::test

#include "flow/unactorcompiler.h"
#endif // FDBSERVER_PTXN_TEST_FAKETLOG_ACTOR_H
