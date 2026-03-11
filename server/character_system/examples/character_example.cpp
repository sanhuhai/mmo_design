#include "character/character_manager.h"
#include "character/character.h"
#include "character/player.h"
#include "character/npc.h"
#include "character/combat_component.h"
#include "character/movement_component.h"
#include "character/network_sync_component.h"
#include <iostream>
#include <thread>
#include <chrono>

using namespace game::character;

void onCharacterCreated(Character* character) {
    std::cout << "Character created: " << character->getName() << std::endl;
}

void onCharacterRemoved(Character* character) {
    std::cout << "Character removed: " << character->getName() << std::endl;
}

void onPlayerDeath() {
    std::cout << "Player died!" << std::endl;
}

void onNPCAggro(uint64_t target_id) {
    std::cout << "NPC aggroed on target: " << target_id << std::endl;
}

void onCharacterMove(const Vector3& position) {
    std::cout << "Character moved to: (" << position.x << ", " << position.y << ", " << position.z << ")" << std::endl;
}

void onSync(const game::character::SyncData& data) {
    std::cout << "Sync data for character: " << data.character_id << std::endl;
}

int main() {
    std::cout << "=== Character System Example ===" << std::endl;
    
    CharacterManager& manager = CharacterManager::getInstance();
    manager.initialize();
    
    manager.setOnCharacterCreatedCallback(onCharacterCreated);
    manager.setOnCharacterRemovedCallback(onCharacterRemoved);
    
    Player* player = manager.createPlayer(1001, "Hero", 10001);
    if (player) {
        auto combat = std::make_unique<CombatComponent>(player);
        combat->setOnDeathCallback(onPlayerDeath);
        player->addComponent(std::move(combat));
        
        auto movement = std::make_unique<MovementComponent>(player);
        movement->setOnMoveCallback(onCharacterMove);
        player->addComponent(std::move(movement));
        
        auto network = std::make_unique<NetworkSyncComponent>(player);
        network->setOnSyncCallback(onSync);
        player->addComponent(std::move(network));
        
        player->getComponent<CombatComponent>()->initialize();
        player->getComponent<MovementComponent>()->initialize();
        player->getComponent<NetworkSyncComponent>()->initialize();
    }
    
    NPC* npc = manager.createNPC(2001, "Goblin", NPCType::MONSTER);
    if (npc) {
        auto combat = std::make_unique<CombatComponent>(npc);
        npc->addComponent(std::move(combat));
        
        auto movement = std::make_unique<MovementComponent>(npc);
        npc->addComponent(std::move(movement));
        
        npc->setSpawnPosition(Vector3(10.0f, 0.0f, 10.0f));
        npc->setAggroRange(15.0f);
        npc->setOnAggroCallback(onNPCAggro);
        
        npc->getComponent<CombatComponent>()->initialize();
        npc->getComponent<MovementComponent>()->initialize();
    }
    
    std::cout << "\n=== Starting Simulation ===" << std::endl;
    std::cout << "Player count: " << manager.getPlayerCount() << std::endl;
    std::cout << "NPC count: " << manager.getNPCCount() << std::endl;
    
    auto player_movement = player->getComponent<MovementComponent>();
    if (player_movement) {
        player_movement->moveTo(Vector3(5.0f, 0.0f, 5.0f));
    }
    
    for (int i = 0; i < 10; ++i) {
        manager.update(0.1f);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    auto player_combat = player->getComponent<CombatComponent>();
    auto npc_combat = npc->getComponent<CombatComponent>();
    
    if (player_combat && npc_combat) {
        std::cout << "\n=== Combat Test ===" << std::endl;
        std::cout << "Player HP: " << player_combat->getAttributes().health << "/" << player_combat->getAttributes().max_health << std::endl;
        std::cout << "NPC HP: " << npc_combat->getAttributes().health << "/" << npc_combat->getAttributes().max_health << std::endl;
        
        player_combat->attack(npc);
        
        std::cout << "After player attack:" << std::endl;
        std::cout << "Player HP: " << player_combat->getAttributes().health << "/" << player_combat->getAttributes().max_health << std::endl;
        std::cout << "NPC HP: " << npc_combat->getAttributes().health << "/" << npc_combat->getAttributes().max_health << std::endl;
        
        npc_combat->attack(player);
        
        std::cout << "After NPC attack:" << std::endl;
        std::cout << "Player HP: " << player_combat->getAttributes().health << "/" << player_combat->getAttributes().max_health << std::endl;
        std::cout << "NPC HP: " << npc_combat->getAttributes().health << "/" << npc_combat->getAttributes().max_health << std::endl;
    }
    
    std::cout << "\n=== Experience Test ===" << std::endl;
    std::cout << "Player level: " << player->getLevel() << std::endl;
    std::cout << "Player experience: " << player->getExperience() << "/" << player->getMaxExperience() << std::endl;
    
    player->addExperience(150);
    
    std::cout << "After adding 150 experience:" << std::endl;
    std::cout << "Player level: " << player->getLevel() << std::endl;
    std::cout << "Player experience: " << player->getExperience() << "/" << player->getMaxExperience() << std::endl;
    
    std::cout << "\n=== Cleanup ===" << std::endl;
    manager.removeCharacter(1001);
    manager.removeCharacter(2001);
    
    std::cout << "Player count: " << manager.getPlayerCount() << std::endl;
    std::cout << "NPC count: " << manager.getNPCCount() << std::endl;
    
    manager.shutdown();
    
    std::cout << "\n=== Example Complete ===" << std::endl;
    
    return 0;
}
