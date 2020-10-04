/**
 * Copyright (c) 2020 Brandon McGriff
 *
 * Licensed under the MIT license; see the LICENSE-src file at the top level
 * directory for the full text of the license.
 */
#pragma once
#include "Settings.h"

namespace Shiro {
    /**
     * Only call this when you're sure you're going to start the game, as it
     * initializes global state and installs a quit function executed on exit.
     */
    void Startup(const Shiro::Settings& settings);
}
