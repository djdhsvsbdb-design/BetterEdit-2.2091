#pragma once

#include <Geode/Result.hpp>
#include <Geode/binding/GJGameLevel.hpp>
#include <asp/fs/Path.hpp>

namespace gmd {
    geode::Result<> exportLevelAsGmd(GJGameLevel* level, asp::fs::PathView to);
    geode::Result<GJGameLevel*> importGmdAsLevel(asp::fs::PathView from);
}
