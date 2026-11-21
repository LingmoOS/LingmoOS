// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ui/display/manager/display_configurator.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: validation and test-only modesetting precede a single
// asynchronous native commit without Ash policy or Chromium task objects.

#include "display/display_configurator.h"


namespace Lingmo::Display {

DisplayConfigurator::DisplayConfigurator(NativeDisplayDelegate *delegate)
    : m_delegate(delegate)
{
    m_hardwareInvalidated = delegate->hardwareInvalidated.connect([this](uint64_t) {
        if (m_busy)
            m_invalidated = true;
    });
}

void DisplayConfigurator::test(const HardwareSnapshot &hardware, uint64_t revision,
                               const DisplayConfiguration &request, Completion completion)
{
    run(hardware, revision, request, false, std::move(completion));
}

void DisplayConfigurator::apply(const HardwareSnapshot &hardware, uint64_t revision,
                                const DisplayConfiguration &request, Completion completion)
{
    run(hardware, revision, request, true, std::move(completion));
}

void DisplayConfigurator::run(const HardwareSnapshot &hardware, uint64_t revision,
                              const DisplayConfiguration &request, bool commit,
                              Completion completion)
{
    if (m_busy) {
        completion({false,
                    false,
                    {DisplayErrorCode::Busy, "A display operation is active", {}},
                    std::nullopt});
        return;
    }

    const auto validated =
            DisplayConfigurationValidator::validateExtended(hardware, revision, request);
    if (!validated.succeeded) {
        completion({false, false, validated.error, std::nullopt});
        return;
    }

    m_busy = true;
    m_invalidated = false;
    const std::weak_ptr<bool> guard = m_alive;
    m_delegate->configure(
            validated.normalized, NativeDisplayDelegate::ConfigureMode::TestOnly,
            [this, guard, hardware, revision, validated, commit,
             completion = std::move(completion)](ConfigurationResult tested) mutable {
                if (guard.expired())
                    return;
                if (m_invalidated) {
                    m_busy = false;
                    completion({false,
                                false,
                                {DisplayErrorCode::StaleConfiguration,
                                 "Display hardware changed during configuration testing",
                                 {}},
                                std::nullopt});
                    return;
                }
                if (!tested.succeeded || !commit) {
                    m_busy = false;
                    completion(std::move(tested));
                    return;
                }
                auto selected = tested.selected.value_or(validated.normalized);
                const auto checked = DisplayConfigurationValidator::validateExtended(
                        hardware, revision, selected);
                if (!checked.succeeded
                    || (tested.selected && !validated.normalized.allowModeFallback)) {
                    m_busy = false;
                    completion({false,
                                false,
                                {DisplayErrorCode::TestFailed,
                                 "Native testing returned an invalid or "
                                                "unauthorized fallback",
                                 {}},
                                std::nullopt});
                    return;
                }
                selected = checked.normalized;
                selected.allowModeFallback = false;
                m_delegate->configure(
                        selected, NativeDisplayDelegate::ConfigureMode::Commit,
                        [this, guard, selected, tested,
                         completion = std::move(completion)](ConfigurationResult result) mutable {
                            if (guard.expired())
                                return;
                            m_busy = false;
                            if (result.succeeded && tested.selected) {
                                result.selected = selected;
                                result.degraded = tested.degraded;
                                result.error = tested.error;
                            }
                            if (m_invalidated)
                                result = {false,
                                          false,
                                          {DisplayErrorCode::StaleConfiguration,
                                           "Display hardware changed during "
                                                          "configuration commit",
                                           {}},
                                          std::nullopt};
                            completion(std::move(result));
                        });
            });
}

} // namespace Lingmo::Display
