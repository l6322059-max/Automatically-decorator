#include <Geode/Geode.hpp>
#include <Geode/modify/EditorUI.hpp>
#include <array>
#include <cmath>
#include <random>
#include <unordered_set>

using namespace geode::prelude;

namespace {
    constexpr float GRID = 30.f;
    constexpr float PI = 3.14159265f;

    int64_t cellKey(CCPoint p) {
        auto cx = static_cast<int32_t>(std::round(p.x / GRID));
        auto cy = static_cast<int32_t>(std::round(p.y / GRID));
        return (static_cast<int64_t>(cx) << 32) | static_cast<uint32_t>(cy);
    }

    std::vector<int> parseIDs(std::string const& s) {
        std::vector<int> out;
        for (auto& part : utils::string::split(s, ",")) {
            auto id = utils::numFromString<int>(utils::string::trim(part));
            if (id.isOk()) out.push_back(id.unwrap());
        }
        return out;
    }

    // Rotates a vector clockwise by deg degrees (matches cocos rotation)
    CCPoint rotateCW(CCPoint v, float deg) {
        float r = deg * PI / 180.f;
        return ccp(
            v.x * std::cos(r) + v.y * std::sin(r),
            -v.x * std::sin(r) + v.y * std::cos(r)
        );
    }
}

class $modify(AutoDecoUI, EditorUI) {
    bool init(LevelEditorLayer* lel) {
        if (!EditorUI::init(lel)) return false;

        auto spr = ButtonSprite::create("Deco");
        spr->setScale(0.6f);
        auto btn = CCMenuItemSpriteExtra::create(
            spr, this, menu_selector(AutoDecoUI::onAutoDeco)
        );
        auto menu = CCMenu::create();
        menu->addChild(btn);
        auto win = CCDirector::get()->getWinSize();
        menu->setPosition(ccp(win.width - 40.f, win.height - 60.f)); // move as you like
        this->addChild(menu, 100);
        return true;
    }

    void onAutoDeco(CCObject*) {
        auto mod = Mod::get();
        auto edgeIDs = parseIDs(mod->getSettingValue<std::string>("edge-ids"));
        auto fillIDs = parseIDs(mod->getSettingValue<std::string>("fill-ids"));
        auto hazardIDs = parseIDs(mod->getSettingValue<std::string>("hazard-ids"));

        if (edgeIDs.empty() && fillIDs.empty() && hazardIDs.empty()) {
            FLAlertLayer::create(
                "Auto Deco", "Add some deco object IDs in the mod settings first.", "OK"
            )->show();
            return;
        }

        float edgeChance = static_cast<float>(mod->getSettingValue<double>("edge-chance"));
        float fillChance = static_cast<float>(mod->getSettingValue<double>("fill-chance"));
        float hazardChance = static_cast<float>(mod->getSettingValue<double>("hazard-chance"));

        auto lel = m_editorLayer;

        static std::mt19937 rng{std::random_device{}()};
        auto rnd = [&](float a, float b) {
            return std::uniform_real_distribution<float>(a, b)(rng);
        };
        auto pick = [&](std::vector<int> const& ids) {
            return ids[rng() % ids.size()];
        };

        // Every solid in the level, so edges next to unselected blocks count as covered
        std::unordered_set<int64_t> solids;
        for (auto obj : CCArrayExt<GameObject*>(lel->m_objects)) {
            if (obj->m_objectType == GameObjectType::Solid) {
                solids.insert(cellKey(obj->getPosition()));
            }
        }

        // Decorate the selection if there is one, otherwise the whole level.
        // Snapshot first, since placing objects changes m_objects.
        std::vector<GameObject*> sources;
        auto sel = this->getSelectedObjects();
        if (sel && sel->count() > 0) {
            for (auto obj : CCArrayExt<GameObject*>(sel)) sources.push_back(obj);
        } else {
            for (auto obj : CCArrayExt<GameObject*>(lel->m_objects)) sources.push_back(obj);
        }

        auto created = CCArray::create();
        auto place = [&](int id, CCPoint pos, float rot, float scale) {
            auto o = lel->createObject(id, pos, false);
            if (!o) return;
            o->setRotation(rot);
            o->setScale(scale);
            created->addObject(o);
        };

        std::array<std::pair<CCPoint, float>, 4> dirs = {{
            {ccp(0, 1), 0.f}, {ccp(1, 0), 90.f}, {ccp(0, -1), 180.f}, {ccp(-1, 0), 270.f}
        }};

        for (auto obj : sources) {
            auto pos = obj->getPosition();

            if (obj->m_objectType == GameObjectType::Solid) {
                // Edge deco: only on faces not touching another solid
                if (!edgeIDs.empty()) {
                    for (auto& [d, rot] : dirs) {
                        auto neighbor = ccp(pos.x + d.x * GRID, pos.y + d.y * GRID);
                        if (solids.count(cellKey(neighbor))) continue;
                        if (rnd(0.f, 1.f) > edgeChance) continue;
                        place(pick(edgeIDs), pos + d * (GRID / 2), rot, 1.f);
                    }
                }
                // Random fill: scattered detail on the block
                if (!fillIDs.empty() && rnd(0.f, 1.f) < fillChance) {
                    place(
                        pick(fillIDs),
                        pos + ccp(rnd(-10.f, 10.f), rnd(-10.f, 10.f)),
                        rnd(0.f, 360.f),
                        rnd(0.7f, 1.1f)
                    );
                }
            }
            else if (obj->m_objectType == GameObjectType::Hazard) {
                // Small deco on both sides of the hazard's base, following its rotation
                if (!hazardIDs.empty() && rnd(0.f, 1.f) < hazardChance) {
                    float rot = obj->getRotation();
                    for (float side : {-1.f, 1.f}) {
                        place(
                            pick(hazardIDs),
                            pos + rotateCW(ccp(side * 12.f, -9.f), rot),
                            rot,
                            0.6f
                        );
                    }
                }
            }
        }

        if (created->count() == 0) {
            Notification::create("Nothing to decorate", NotificationIcon::Warning)->show();
            return;
        }

        // One undo entry for the whole batch
        lel->m_undoObjects->addObject(UndoObject::createWithArray(created, UndoCommand::Paste));

        Notification::create(
            fmt::format("Placed {} deco objects", created->count()),
            NotificationIcon::Success
        )->show();
    }
};
