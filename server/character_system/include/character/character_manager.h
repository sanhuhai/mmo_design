#ifndef CHARACTER_MANAGER_H
#define CHARACTER_MANAGER_H

#include "character/character.h"
#include "character/player.h"
#include "character/npc.h"
#include <memory>
#include <unordered_map>
#include <vector>
#include <functional>
#include <mutex>

namespace game {
namespace character {

class CharacterManager {
public:
    static CharacterManager& getInstance();

    void initialize();
    void shutdown();
    void update(float delta_time);

    Character* createCharacter(uint64_t id, const std::string& name, CharacterType type);
    Player* createPlayer(uint64_t id, const std::string& name, uint64_t account_id);
    NPC* createNPC(uint64_t id, const std::string& name, NPCType npc_type);

    void removeCharacter(uint64_t id);
    void removeAllCharacters();

    Character* getCharacter(uint64_t id);
    Player* getPlayer(uint64_t id);
    NPC* getNPC(uint64_t id);

    const std::vector<Character*>& getAllCharacters() const { return characters_; }
    const std::vector<Player*>& getPlayers() const { return players_; }
    const std::vector<NPC*>& getNPCs() const { return npcs_; }

    std::vector<Character*> getCharactersInRange(const Vector3& position, float range) const;
    std::vector<Player*> getPlayersInRange(const Vector3& position, float range) const;
    std::vector<NPC*> getNPCsInRange(const Vector3& position, float range) const;

    void setOnCharacterCreatedCallback(std::function<void(Character*)> callback) { on_character_created_callback_ = callback; }
    void setOnCharacterRemovedCallback(std::function<void(Character*)> callback) { on_character_removed_callback_ = callback; }

    size_t getCharacterCount() const { return characters_.size(); }
    size_t getPlayerCount() const { return players_.size(); }
    size_t getNPCCount() const { return npcs_.size(); }

private:
    CharacterManager();
    ~CharacterManager() = default;
    CharacterManager(const CharacterManager&) = delete;
    CharacterManager& operator=(const CharacterManager&) = delete;

private:
    std::unordered_map<uint64_t, std::unique_ptr<Character>> characters_map_;
    std::vector<Character*> characters_;
    std::vector<Player*> players_;
    std::vector<NPC*> npcs_;

    mutable std::mutex mutex_;

    std::function<void(Character*)> on_character_created_callback_;
    std::function<void(Character*)> on_character_removed_callback_;
};

} 
} 

#endif 
