/*
 * TesterCorrectnessWorkload.cpp
 *
 * This source file is part of the FoundationDB open source project
 *
 * Copyright 2013-2022 Apple Inc. and the FoundationDB project authors
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
#include "TesterWorkload.h"
#include <memory>
#include <optional>
#include <iostream>

namespace FdbApiTester {

class ApiCorrectnessWorkload : public WorkloadBase {
public:
	ApiCorrectnessWorkload() : numTxLeft(1000) {}

	void start() override {
		schedule([this]() { nextTransaction(); });
	}

private:
	void nextTransaction() {
		if (numTxLeft % 100 == 0) {
			std::cout << numTxLeft << " transactions left" << std::endl;
		}
		if (numTxLeft == 0)
			return;

		numTxLeft--;
		execTransaction(
		    [](auto ctx) {
			    ValueFuture fGet = ctx->tx()->get(ctx->dbKey("foo"), false);
			    ctx->continueAfter(fGet, [fGet, ctx]() {
				    std::optional<std::string_view> optStr = fGet.getValue();
				    ctx->tx()->set(ctx->dbKey("foo"), optStr.value_or("bar"));
				    ctx->commit();
			    });
		    },
		    [this]() { nextTransaction(); });
	}

	int numTxLeft;
};

std::shared_ptr<IWorkload> createApiCorrectnessWorkload() {
	return std::make_shared<ApiCorrectnessWorkload>();
}

} // namespace FdbApiTester