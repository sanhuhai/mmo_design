#include "character/character_manager.h"
#include "character/player.h"
#include "character/npc.h"
#include "character/combat_component.h"
#include "character/movement_component.h"
#include "character/network_sync_component.h"
#include "character/component.h"
#include <iostream>
#include <thread>
#include <chrono>

using namespace game::character;

void onCharacterCreated(Character* character) {
    std::cout << "Character created: " << character->getName() 
              << " (ID: " << character->getId() << ")" << std::endl;
}

void onCharacterRemoved(Character* character) {
    std::cout << "Character removed: " << character->getName() 
              << " (ID: " << character->getId() << ")" << std::endl;
}

void onPlayerDeath() {
    std::cout << "Player has died!" << std::endl;
}

void onCharacterMove(const Vector3& position) {
    std::cout << "Character moved to: (" << position.x << ", " 
              << position.y << ", " << position.z << ")" << std::endl;
}

void onSync(const game::character::SyncData& data) {
    std::cout << "Sync data for character " << data.character_id << std::endl;
}

int main() {
    std::cout << "=== Component System Test ===" << std::endl;
    
    CharacterManager& manager = CharacterManager::getInstance();
    manager.initialize();
    
    manager.setOnCharacterCreatedCallback(onCharacterCreated);
    manager.setOnCharacterRemovedCallback(onCharacterRemoved);
    
    std::cout << "\n--- Creating Player with Components ---" << std::endl;
    Player* player = manager.createPlayer(1001, "Hero", 10001);
    if (player) {
        auto combat = std::make_unique<CombatComponent>(player);
        combat->setOnDeathCallback(onPlayerDeath);
        combat->setPriority(ComponentPriority::HIGH);
        player->addComponent(std::move(combat));
        
        auto movement = std::make_unique<MovementComponent>(player);
        movement->setOnMoveCallback(onCharacterMove);
        movement->setPriority(ComponentPriority::NORMAL);
        player->addComponent(std::move(movement));
        
        auto network = std::make_unique<NetworkSyncComponent>(player);
        network->setOnSyncCallback(onSync);
        network->setPriority(ComponentPriority::CRITICAL);
        player->addComponent(std::move(network));
        
        std::cout << "Player components added successfully" << std::endl;
    }
    
    std::cout << "\n--- Creating NPC with Components ---" << std::endl;
    NPC* npc = manager.createNPC(2001, "Goblin", NPCType::MONSTER);
    if (npc) {
        auto combat = std::make_unique<CombatComponent>(npc);
        combat->setPriority(ComponentPriority::NORMAL);
        npc->addComponent(std::move(combat));
        
        auto movement = std::make_unique<MovementComponent>(npc);
        movement->setPriority(ComponentPriority::NORMAL);
        npc->addComponent(std::move(movement));
        
        std::cout << "NPC components added successfully" << std::endl;
    }
    
    std::cout << "\n--- Testing Component State Management ---" << std::endl;
    if (player) {
        auto combat = player->getComponent<CombatComponent>();
        if (combat) {
            std::cout << "Combat Component State: " << static_cast<int>(combat->getState()) << std::endl;
            std::cout << "Combat Component Enabled: " << (combat->isEnabled() ? "true" : "false") << std::endl;
            std::cout << "Combat Component Priority: " << static_cast<int>(combat->getPriority()) << std::endl;
            std::cout << "Combat Component Type: " << combat->getTypeName() << std::endl;
            std::cout << "Combat Component ID: " << combat->getComponentId() << std::endl;
        }
        
        auto movement = player->getComponent<MovementComponent>();
        if (movement) {
            std::cout << "Movement Component State: " << static_cast<int>(movement->getState()) << std::endl;
            std::cout << "Movement Component Enabled: " << (movement->isEnabled() ? "true" : "false") << std::endl;
        }
    }
    
    std::cout << "\n--- Testing Component Enable/Disable ---" << std::endl;
    if (player) {
        auto combat = player->getComponent<CombatComponent>();
        if (combat) {
            combat->setEnabled(false);
            std::cout << "Combat component disabled" << std::endl;
            std::cout << "Combat Component Enabled: " << (combat->isEnabled() ? "true" : "false") << std::endl;
            
            combat->setEnabled(true);
            std::cout << "Combat component re-enabled" << std::endl;
            std::cout << "Combat Component Enabled: " << (combat->isEnabled() ? "true" : "false") << std::endl;
        }
    }
    
    std::cout << "\n--- Testing Component Pause/Resume ---" << std::endl;
    if (player) {
        auto movement = player->getComponent<MovementComponent>();
        if (movement) {
            movement->setPaused(true);
            std::cout << "Movement component paused" << std::endl;
            std::cout << "Movement Component Paused: " << (movement->isPaused() ? "true" : "false") << std::endl;
            
            movement->setPaused(false);
            std::cout << "Movement component resumed" << std::endl;
            std::cout << "Movement Component Paused: " << (movement->isPaused() ? "true" : "false") << std::endl;
        }
    }
    
    std::cout << "\n--- Testing Component Events ---" << std::endl;
    if (player) {
        auto combat = player->getComponent<CombatComponent>();
        if (combat) {
            combat->addEventListener("onAttack", [](const ComponentEvent& event) {
                std::cout << "Attack event triggered!" << std::endl;
            });
            
            combat->triggerEvent("onAttack");
            
            combat->removeEventListener("onAttack");
            std::cout << "Event listener removed" << std::endl;
        }
    }
    
    std::cout << "\n--- Testing Component Dependencies ---" << std::endl;
    if (player) {
        auto combat = player->getComponent<CombatComponent>();
        if (combat) {
            combat->setDependency("MovementComponent");
            std::cout << "Added dependency: MovementComponent" << std::endl;
            std::cout << "Has dependency MovementComponent: " 
                      << (combat->hasDependency("MovementComponent") ? "true" : "false") << std::endl;
        }
    }
    
    std::cout << "\n--- Testing Component Dirty Flag ---" << std::endl;
    if (player) {
        auto combat = player->getComponent<CombatComponent>();
        if (combat) {
            std::cout << "Combat component dirty: " << (combat->isDirty() ? "true" : "false") << std::endl;
            combat->setDirty(true);
            std::cout << "Set combat component dirty" << std::endl;
            std::cout << "Combat component dirty: " << (combat->isDirty() ? "true" : "false") << std::endl;
        }
    }
    
    std::cout << "\n--- Testing Custom Data ---" << std::endl;
    if (player) {
        auto combat = player->getComponent<CombatComponent>();
        if (combat) {
            int custom_value = 42;
            combat->setCustomData("test_value", &custom_value);
            std::cout << "Set custom data: test_value = 42" << std::endl;
            
            void* data = combat->getCustomData("test_value");
            if (data) {
                int* value = static_cast<int*>(data);
                std::cout << "Get custom data: test_value = " << *value << std::endl;
            }
            
            combat->removeCustomData("test_value");
            std::cout << "Removed custom data" << std::endl;
        }
    }
    
    std::cout << "\n--- Testing Component Debug Print ---" << std::endl;
    if (player) {
        auto combat = player->getComponent<CombatComponent>();
        if (combat) {
            std::cout << "\nCombat Component Debug Info:" << std::endl;
            combat->debugPrint();
        }
    }
    
    std::cout << "\n--- Running Game Loop (5 iterations) ---" << std::endl;
    for (int i = 0; i < 5; ++i) {
        std::cout << "\nUpdate " << (i + 1) << ":" << std::endl;
        manager.update(0.1f);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    std::cout << "\n--- Testing Player Level Up ---" << std::endl;
    if (player) {
        std::cout << "Player Level: " << player->getLevel() << std::endl;
        std::cout << "Player Experience: " << player->getExperience() << std::endl;
        std::cout << "Player Max Experience: " << player->getMaxExperience() << std::endl;
        
        player->addExperience(200);
        
        std::cout << "After adding 200 experience:" << std::endl;
        std::cout << "Player Level: " << player->getLevel() << std::endl;
        std::cout << "Player Experience: " << player->getExperience() << std::endl;
        std::cout << "Player Max Experience: " << player->getMaxExperience() << std::endl;
    }
    
    std::cout << "\n--- Testing Player Economy ---" << std::endl;
    if (player) {
        std::cout << "Player Gold: " << player->getGold() << std::endl;
        std::cout << "Player Diamond: " << player->getDiamond() << std::endl;
        
        player->addGold(100);
        player->addDiamond(10);
        
        std::cout << "After adding currency:" << std::endl;
        std::cout << "Player Gold: " << player->getGold() << std::endl;
        std::cout << "Player Diamond: " << player->getDiamond() << std::endl;
    }
    
    std::cout << "\n--- Testing NPC AI ---" << std::endl;
    if (npc) {
        std::cout << "NPC Name: " << npc->getName() << std::endl;
        std::cout << "NPC AI State: " << static_cast<int>(npc->getAIState()) << std::endl;
        std::cout << "NPC Aggro Range: " << npc->getAggroRange() << std::endl;
        
        npc->setAIState(AIState::PATROL);
        std::cout << "Set NPC AI State to PATROL" << std::endl;
        std::cout << "NPC AI State: " << static_cast<int>(npc->getAIState()) << std::endl;
    }
    
    std::cout << "\n--- Cleanup ---" << std::endl;
    manager.removeCharacter(1001);
    manager.removeCharacter(2001);
    
    manager.shutdown();
    
    std::cout << "\n=== Test Complete ===" << std::endl;
    return 0;
}
