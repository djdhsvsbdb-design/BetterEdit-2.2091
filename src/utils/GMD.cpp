#include "GMD.hpp"

#include <Geode/utils/file.hpp>
#include <Geode/mapping/DS_Dictionary.hpp>
#include <algorithm>
#include <memory>
#include <string>

using namespace geode::prelude;

namespace gmd {
    static void normalizeLevelData(std::string& data) {
        std::replace(data.begin(), data.end(), '\0', ' ');

        const auto plistTag = "<plist version=\"1.0\">";
        if (const auto pos = data.substr(0, 100).find(plistTag); pos != std::string::npos) {
            data.replace(pos, sizeof("<plist version=\"1.0\">") - 1, "<plist version=\"1.0\" gjver=\"2.0\">");
        }

        if (data.starts_with("<?xml version")) {
            return;
        }

        if (data.substr(0, 100).find("<plist version") == std::string::npos) {
            data = "<?xml version=\"1.0\"?><plist version=\"1.0\" gjver=\"2.0\"><dict><k>root</k>" +
                data + "</dict></plist>";
        }
        else {
            data.insert(0, "<?xml version=\"1.0\"?>");
        }
    }

    Result<> exportLevelAsGmd(GJGameLevel* level, asp::fs::PathView to) {
        if (!level) {
            return Err("Cannot export a null level");
        }

        auto dict = std::make_unique<DS_Dictionary>();
        level->encodeWithCoder(dict.get());
        const auto data = std::string(dict->saveRootSubDictToString());
        return file::writeString(to, data);
    }

    Result<GJGameLevel*> importGmdAsLevel(asp::fs::PathView from) {
        GEODE_UNWRAP_INTO(auto data, file::readString(from));
        normalizeLevelData(data);

        auto dict = std::make_unique<DS_Dictionary>();
        if (!dict->loadRootSubDictFromString(data)) {
            return Err("Unable to parse level data");
        }
        dict->stepIntoSubDictWithKey("root");

        auto level = GJGameLevel::create();
        level->dataLoaded(dict.get());
        level->m_isEditable = true;
        level->m_levelType = GJLevelType::Editor;

        if (level->m_levelString.empty()) {
            level->m_levelString = dict->getStringForKey("k4");
        }

        return Ok(level);
    }
}
