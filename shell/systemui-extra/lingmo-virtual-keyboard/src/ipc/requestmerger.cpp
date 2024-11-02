/*
 * Copyright 2022 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "requestmerger.h"

class RequestMerger::State {
public:
    virtual void activate() const = 0;
    virtual void deactivate() const = 0;
    void execute() const {
        requestMerger_.enterIdleState();

        if (!executeCallback_) {
            return;
        }

        executeCallback_();
    }

protected:
    State(RequestMerger &requestMerger, ExecuteCallback executeCallback)
        : requestMerger_(requestMerger),
          executeCallback_(std::move(executeCallback)) {}
    virtual ~State() = default;

protected:
    RequestMerger &requestMerger_;

private:
    ExecuteCallback executeCallback_;
};

class RequestMerger::IdleState : public RequestMerger::State {
public:
    IdleState(RequestMerger &requestMerger,
              ShouldActivateCallback shouldActivateCallback,
              ShouldDeactivateCallback shouldDeactivateCallback)
        : State(requestMerger, ExecuteCallback()),
          shouldActivateCallback_(std::move(shouldActivateCallback)),
          shouldDeactivateCallback_(std::move(shouldDeactivateCallback)) {}
    ~IdleState() override = default;

    void activate() const override {
        if (!shouldActivate()) {
            return;
        }

        requestMerger_.enterActivationState();
    }

    void deactivate() const override {
        if (!shouldDeactivate()) {
            return;
        }

        requestMerger_.enterDeactivationState();
    }

private:
    bool shouldActivate() const {
        return shouldActivateCallback_ && shouldActivateCallback_();
    }
    bool shouldDeactivate() const {
        return shouldDeactivateCallback_ && shouldDeactivateCallback_();
    }

private:
    ShouldActivateCallback shouldActivateCallback_;
    ShouldDeactivateCallback shouldDeactivateCallback_;
};

class RequestMerger::ActivationState : public RequestMerger::State {
public:
    ActivationState(RequestMerger &requestMerger,
                    ExecuteCallback executeCallback)
        : State(requestMerger, std::move(executeCallback)) {}
    ~ActivationState() override = default;

    void activate() const override {}

    void deactivate() const override { requestMerger_.enterIdleState(); }
};

class RequestMerger::DeactivationState : public RequestMerger::State {
public:
    DeactivationState(RequestMerger &requestMerger,
                      ExecuteCallback executeCallback)
        : State(requestMerger, std::move(executeCallback)) {}
    ~DeactivationState() override = default;

    void activate() const override { requestMerger_.enterIdleState(); }

    void deactivate() const override {}
};

RequestMerger::RequestMerger(int period) {
    period_ = std::max(LEAST_PERIOD, period);

    timer_.setSingleShot(true);

    QObject::connect(&timer_, &QTimer::timeout, this, &RequestMerger::execute);
}

RequestMerger::~RequestMerger() = default;

void RequestMerger::init(ExecuteCallback activationExecuteCallback,
                         ExecuteCallback deactivationExecuteCallback,
                         ShouldActivateCallback shouldAcitvateCallback,
                         ShouldDeactivateCallback shouldDeactivateCallback) {
    idleState_ =
        std::make_shared<IdleState>(*this, std::move(shouldAcitvateCallback),
                                    std::move(shouldDeactivateCallback));
    activationState_ = std::make_shared<ActivationState>(
        *this, std::move(activationExecuteCallback));
    deactivationState_ = std::make_shared<DeactivationState>(
        *this, std::move(deactivationExecuteCallback));

    updateCurrentState(idleState_);
}

void RequestMerger::activate() { currentState_->activate(); }

void RequestMerger::deactivate() { currentState_->deactivate(); }

void RequestMerger::start() { timer_.start(period_); }

void RequestMerger::stop() { timer_.stop(); }

void RequestMerger::updateCurrentState(std::shared_ptr<State> newState) {
    currentState_ = newState;
}

void RequestMerger::enterIdleState() {
    updateCurrentState(idleState_);
    stop();
}
void RequestMerger::enterActivationState() {
    updateCurrentState(activationState_);
    start();
}
void RequestMerger::enterDeactivationState() {
    updateCurrentState(deactivationState_);
    start();
}

void RequestMerger::execute() { currentState_->execute(); }
