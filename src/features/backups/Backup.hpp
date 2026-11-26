#pragma once

#include <Geode/binding/GJGameLevel.hpp>
#include <Geode/utils/cocos.hpp>
#include <asp/fs/Path.hpp>

using namespace geode::prelude;

asp::fs::Path getBackupsDir(GJGameLevel* level);

class Backup final {
public:
    using Clock = std::chrono::system_clock;
    using TimePoint = std::chrono::time_point<Clock>;
    using TimeUnit = std::chrono::minutes;

private:
    asp::fs::Path m_directory;
    Ref<GJGameLevel> m_level;
    Ref<GJGameLevel> m_forLevel;
    TimePoint m_createTime;
    bool m_automated;

public:
    static Result<std::shared_ptr<Backup>> load(asp::fs::Path const& dir, GJGameLevel* forLevel);
    static std::vector<std::shared_ptr<Backup>> load(GJGameLevel* level);
    static Result<> create(GJGameLevel* level, bool automated);
    static Result<> cleanAutomated(GJGameLevel* level);

    GJGameLevel* getLevel() const;
    GJGameLevel* getOriginalLevel() const;
    TimePoint getCreateTime() const;
    bool isAutomated() const;

    Result<> restoreThis();
    Result<> deleteThis();
    Result<> preserveAutomated();
};
using BackupPtr = std::shared_ptr<Backup>;
