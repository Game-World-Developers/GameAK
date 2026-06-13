#pragma once

#include "gameak/core/Result.h"

#include <functional>

namespace gameak::runtime {

class Runtime;
class Command;
class StateView;
class CommandProducer;

using Controller = std::function<core::Result<void>(StateView&, CommandProducer&)>;

class StateView {
public:
    explicit StateView(const Runtime& runtime)
        : runtime_{runtime} {}

    const Runtime& runtime() const { return runtime_; }

private:
    const Runtime& runtime_;
};

class CommandProducer {
public:
    explicit CommandProducer(Runtime& runtime)
        : runtime_{runtime} {}

    core::Result<void> produce(Command command);

private:
    Runtime& runtime_;
};

} // namespace gameak::runtime
