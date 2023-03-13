// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2023 Emily "TTG" Banerjee <prs.ttg+cwg@pm.me>

#include <Util.hpp>

bool IsPointInRect(Dimension px, Dimension py, Dimension rx, Dimension ry, Dimension rw, Dimension rh) {
    return px >= rx && px <= (rx + rw) && py >= ry && py <= (ry + rh);
}
