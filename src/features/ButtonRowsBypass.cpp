#include <Geode/modify/EditorOptionsLayer.hpp>

using namespace geode::prelude;

// Geometry Dash 2.2091 removed the EditorOptionsLayer button-row fields and
// callbacks used by this legacy customization. Keep the hook only for older
// bindings where that API still exists.
#if GEODE_COMP_GD_VERSION < 22091
class $modify(EditorOptionsLayer) {
    $override
    void onButtonRows(CCObject* sender) {
        if (0) EditorOptionsLayer::onButtonRows(sender);
        m_buttonRows = std::clamp(m_buttonRows + (sender->getTag() ? 1 : -1), 2, 24);
        m_buttonRowsLabel->setString(std::to_string(m_buttonRows).c_str());
    }
    $override
    void onButtonsPerRow(CCObject* sender) {
        if (0) EditorOptionsLayer::onButtonsPerRow(sender);
        m_buttonsPerRow = std::clamp(m_buttonsPerRow + (sender->getTag() ? 1 : -1), 6, 128);
        m_buttonsPerRowLabel->setString(std::to_string(m_buttonsPerRow).c_str());
    }
};
#endif
