#include <Geode/modify/EditorUI.hpp>
#include <Geode/modify/GameObject.hpp>
#include <Geode/modify/DrawGridLayer.hpp>
#include <Geode/modify/LevelEditorLayer.hpp>
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/EditButtonBar.hpp>
#include <Geode/binding/GameManager.hpp>
#include <Geode/binding/GJGroundLayer.hpp>
#include <Geode/utils/cocos.hpp>
#include <utils/BEMenuItemToggler.hpp>
#include <utils/Editor.hpp>
#include <utils/HolyUB.hpp>
#include <features/supporters/Pro.hpp>
#include <alphalaneous.editortab_api/API.hpp>

using namespace geode::prelude;

class $modify(GameObjectExtra, GameObject) {
    $override
    void updateVisibility() {
        this->setVisible(m_bVisible || !this->shouldHide());
    }

    $override
    void setVisible(bool visible) {
        if (this->shouldHide()) {
            visible = false;
        }
        GameObject::setVisible(visible);
    }

    bool shouldHide() const {
        return
            m_isHighDetail &&
            EditorUI::get() &&
            Mod::get()->template getSavedValue<bool>("hide-ldm", false);
    }
};

struct $modify(ViewTabUI, EditorUI) {
    static void onModify(auto& self) {
        (void)self.setHookPriority("EditorUI::init", Priority::Early);
        (void)self.setHookPriority("EditorUI::selectObject", Priority::VeryLate);
        (void)self.setHookPriority("EditorUI::selectObjects", Priority::VeryLate);
    }

    ButtonSprite* createViewToggleSpr(const char* frame, bool selected) {
        auto top = CCSprite::createWithSpriteFrameName(frame);
        return ButtonSprite::create(
            top, 50, 0, 50, .8f, true,
            (selected ? "GJ_button_02.png" : "GJ_button_01.png"), true, 0.f
        );
    }

    BEMenuItemToggler* createViewToggle(
        const char* frame,
        std::function<bool()> get,
        std::function<void(bool)> set,
        std::function<bool()> shouldEnable = nullptr
    ) {
        auto off = createViewToggleSpr(frame, false);
        auto on = createViewToggleSpr(frame, true);
        return BEMenuItemToggler::create(off, on, get, set, shouldEnable);
    }

    BEMenuItemToggler* createViewToggleGV(
        const char* frame, const char* gv,
        std::function<bool()> shouldEnable = nullptr
    ) {
        return this->createViewToggle(
            frame,
            [gv]() { return GameManager::get()->getGameVariable(gv); },
            [this, gv](bool enabled) {
                GameManager::get()->setGameVariable(gv, enabled);
                m_editorLayer->updateOptions();
                static_cast<ViewTabUI*>(EditorUI::get())->updateViewTab();
            },
            shouldEnable
        );
    }

    BEMenuItemToggler* createViewToggleMSV(
        const char* frame, const char* modSavedValue,
        bool defaultValue = false, std::function<bool()> shouldEnable = nullptr
    ) {
        return this->createViewToggle(
            frame,
            [modSavedValue, defaultValue]() {
                return Mod::get()->template getSavedValue<bool>(modSavedValue, defaultValue);
            },
            [modSavedValue](bool enabled) {
                Mod::get()->setSavedValue(modSavedValue, enabled);
                static_cast<ViewTabUI*>(EditorUI::get())->updateViewTab();
            },
            shouldEnable
        );
    }

    BEMenuItemToggler* createViewToggleMS(
        const char* frame, const char* modSettingKey,
        std::function<bool()> shouldEnable = nullptr
    ) {
        return this->createViewToggle(
            frame,
            [modSettingKey]() { return Mod::get()->getSettingValue<bool>(modSettingKey); },
            [modSettingKey](bool enabled) {
                Mod::get()->setSettingValue(modSettingKey, enabled);
                static_cast<ViewTabUI*>(EditorUI::get())->updateViewTab();
            },
            shouldEnable
        );
    }

    void updateViewTab() {
        if (auto tab = alpha::editor_tabs::getTab("hjfod.betteredit/view"_spr)) {
            if (auto node = tab->getNode()) {
                for (auto toggle : CCArrayExt<BEMenuItemToggler*>(node->getItems())) {
                    toggle->updateState();
                }
            }
        }
    }

    void setupViewTab() {
        auto viewMode = alpha::editor_tabs::getMode(alpha::editor_tabs::View);
        if (!viewMode) {
            log::error("BetterEdit: Editor Tab API did not create the View mode");
            return;
        }

        std::vector<Ref<CCNode>> btns;

        btns.push_back(this->createViewToggle(
            "v-rotation.png"_spr,
            [] { return GameManager::get()->getGameVariable("0118"); },
            [this](bool) {
                fakeEditorPauseLayer(m_editorLayer)->togglePreviewAnim(nullptr);
            }
        ));
        btns.push_back(this->createViewToggle(
            "v-particles.png"_spr,
            [] { return GameManager::get()->getGameVariable("0117"); },
            [this](bool) { m_editorLayer->updatePreviewParticles(); }
        ));
        btns.push_back(this->createViewToggleGV("v-shaders.png"_spr, "0158"));
        btns.push_back(this->createViewToggle(
            "v-ldm.png"_spr,
            [] { return Mod::get()->getSavedValue("hide-ldm", false); },
            [this](bool) {
                for (auto obj : CCArrayExt<GameObjectExtra*>(m_editorLayer->m_objects)) {
                    obj->updateVisibility();
                }
            }
        ));
        btns.push_back(this->createViewToggle(
            "v-preview-mode.png"_spr,
            [] { return GameManager::get()->getGameVariable("0036"); },
            [this](bool) {
                if (m_editorLayer->m_playbackMode != PlaybackMode::Not) {
                    this->onStopPlaytest(m_playtestBtn);
                }
                m_editorLayer->updateEditorMode();
            }
        ));
        btns.push_back(this->createViewToggle(
            "v-bpm-lines.png"_spr,
            [] { return GameManager::get()->m_showSongMarkers; },
            [](bool enable) { GameManager::get()->m_showSongMarkers = enable; }
        ));
        btns.push_back(this->createViewToggleMSV("v-position-line.png"_spr, "pos-line"));
        btns.push_back(this->createViewToggleGV("v-duration-lines.png"_spr, "0058"));
        btns.push_back(this->createViewToggleGV("v-effect-lines.png"_spr, "0043"));
        btns.push_back(this->createViewToggle(
            "v-ground.png"_spr,
            [] { return GameManager::get()->getGameVariable("0037"); },
            [this](bool enable) { m_editorLayer->m_groundLayer->setVisible(enable); }
        ));
        btns.push_back(this->createViewToggleGV("v-grid.png"_spr, "0038"));
        btns.push_back(this->createViewToggleMSV("v-dash-lines.png"_spr, "show-dash-lines"));
        btns.push_back(this->createViewToggleGV("v-hitboxes.png"_spr, "0045"));

        auto indicatorsToggle = this->createViewToggleMS("v-indicators.png"_spr, "trigger-indicators");
        btns.push_back(indicatorsToggle);
        btns.push_back(this->createViewToggleMSV(
            "v-indicators-trigger-to-trigger.png"_spr,
            "trigger-indicators-trigger-to-trigger",
            false,
            [indicatorsToggle] { return indicatorsToggle->isToggled(); }
        ));

        auto showAll = this->createViewToggleMSV(
            "v-indicators-all.png"_spr,
            "trigger-indicators-show-all",
            false,
            [indicatorsToggle] { return indicatorsToggle->isToggled(); }
        );
        btns.push_back(showAll);
        btns.push_back(this->createViewToggleMSV(
            "v-indicators-cluster-outline.png"_spr,
            "trigger-indicators-cluster-outlines",
            false,
            [indicatorsToggle, showAll] {
                return indicatorsToggle->isToggled() && showAll->isToggled();
            }
        ));
        btns.push_back(this->createViewToggleMSV(
            "v-indicators-blocky.png"_spr,
            "trigger-indicators-blocky",
            false,
            [indicatorsToggle] { return indicatorsToggle->isToggled(); }
        ));

        auto node = alpha::editor_tabs::EditorTab::create(
            std::span<Ref<CCNode>>(btns.data(), btns.size())
        );
        auto icon = CCSprite::createWithSpriteFrameName("tab-view.png"_spr);
        viewMode->createTab("hjfod.betteredit/view"_spr, node, icon);
        this->updateViewTab();
    }

    $override
    bool init(LevelEditorLayer* lel) {
        const bool enableViewTab = Mod::get()->template getSettingValue<bool>("view-menu");
        if (!EditorUI::init(lel)) {
            return false;
        }

        if (enableViewTab) {
            this->setupViewTab();
            this->addEventListener(
                KeybindSettingPressedEventV3(Mod::get(), "keybind-view-mode"),
                [this](Keybind const&, bool down, bool, double) {
                    if (down && m_editorLayer->m_playbackMode == PlaybackMode::Not) {
                        alpha::editor_tabs::switchMode(alpha::editor_tabs::View);
                    }
                }
            );
        }

        return true;
    }

    $override
    void selectObject(GameObject* obj, bool filter) {
        if (!static_cast<GameObjectExtra*>(obj)->shouldHide()) {
            EditorUI::selectObject(obj, filter);
        }
    }

    $override
    void selectObjects(CCArray* objs, bool ignoreFilters) {
        if (objs) {
            unsigned int objCount = objs->count();
            for (unsigned int i = 0; i < objCount; i++) {
                auto obj = static_cast<GameObjectExtra*>(objs->objectAtIndex(i));
                if (obj->shouldHide()) {
                    objs->removeObjectAtIndex(i, false);
                    --i;
                    --objCount;
                }
            }
        }
        EditorUI::selectObjects(objs, ignoreFilters);
    }
};

class $modify(DrawGridLayer) {
    $override
    void draw() {
        bool origBool = m_editorLayer->m_previewMode;
        m_editorLayer->m_previewMode = Mod::get()->template getSavedValue<bool>("pos-line");
        DrawGridLayer::draw();
        m_editorLayer->m_previewMode = origBool;
    }
};

class $modify(LevelEditorLayer) {
    void updateOptions() {
        LevelEditorLayer::updateOptions();
        if (m_editorUI) {
            static_cast<ViewTabUI*>(m_editorUI)->updateViewTab();
        }
    }
};
