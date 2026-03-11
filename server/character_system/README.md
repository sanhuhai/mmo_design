# Character System - 游戏角色系统

一个基于ECS（Entity-Component-System）架构的C++游戏角色系统，专为MMO游戏设计。

## 项目简介

本项目提供了一个完整的游戏角色系统框架，包含基础角色类、玩家类、NPC类以及各种功能组件。采用组件化设计，支持灵活的角色定制和扩展。

## 核心特性

- **组件化架构**：基于ECS设计，角色由多个组件组成
- **基础角色系统**：包含基础属性、成长属性、派生属性
- **战斗组件**：战斗状态机、技能系统、伤害计算、仇恨管理
- **移动组件**：位置管理、路径查找、移动控制
- **网络同步**：状态同步、RPC系统、预测与回滚
- **玩家系统**：等级、经验、金币、钻石管理
- **NPC系统**：AI行为、巡逻、追击、仇恨系统
- **角色管理器**：统一管理所有角色实例

## 项目结构

```
character_system/
├── include/character/       # 头文件目录
│   ├── character.h          # 基础角色类
│   ├── character_types.h    # 角色类型定义
│   ├── component.h         # 组件基类
│   ├── combat_component.h  # 战斗组件
│   ├── movement_component.h # 移动组件
│   ├── network_sync_component.h # 网络同步组件
│   ├── player.h           # 玩家类
│   ├── npc.h             # NPC类
│   └── character_manager.h # 角色管理器
├── src/character/          # 源文件目录
│   ├── character.cpp
│   ├── combat_component.cpp
│   ├── movement_component.cpp
│   ├── network_sync_component.cpp
│   ├── player.cpp
│   ├── npc.cpp
│   └── character_manager.cpp
├── examples/               # 示例代码
│   └── character_example.cpp
└── CMakeLists.txt         # CMake构建配置
```

## 核心类说明

### Character（基础角色）

所有角色的基类，提供通用的角色功能：

- **属性管理**：基础属性、成长属性、派生属性
- **组件系统**：动态添加/移除组件
- **生命周期**：初始化、更新、死亡处理

```cpp
Character* character = new Character(1001, "Hero", CharacterType::PLAYER);
character->addComponent(std::make_unique<CombatComponent>(character));
character->addComponent(std::make_unique<MovementComponent>(character));
```

### Player（玩家）

继承自Character，添加玩家特有的功能：

- **等级系统**：经验值获取、升级处理
- **经济系统**：金币、钻石管理
- **数据持久化**：保存/加载玩家数据
- **在线状态**：登录/登出时间记录

```cpp
Player* player = manager.createPlayer(1001, "Hero", 10001);
player->addExperience(150);
player->addGold(100);
player->setOnline(true);
```

### NPC（系统NPC）

继承自Character，添加AI行为：

- **AI状态机**：空闲、巡逻、追击、攻击、逃跑
- **仇恨系统**：目标锁定、仇恨值管理
- **巡逻系统**：随机巡逻、路径跟随
- **重生系统**：死亡后自动重生

```cpp
NPC* npc = manager.createNPC(2001, "Goblin", NPCType::MONSTER);
npc->setAggroRange(15.0f);
npc->setSpawnPosition(Vector3(10.0f, 0.0f, 10.0f));
```

### 组件系统

#### CombatComponent（战斗组件）

处理所有战斗相关逻辑：

- **战斗状态**：空闲、移动、攻击、施法、受击、死亡
- **技能系统**：技能管理、冷却时间、施法判定
- **伤害计算**：物理/魔法伤害、暴击、护甲穿透
- **仇恨管理**：目标选择、仇恨值衰减

```cpp
auto combat = character->getComponent<CombatComponent>();
combat->attack(target);
combat->castSkill(1001, target);
combat->takeDamage(damage_info);
```

#### MovementComponent（移动组件）

处理角色移动和导航：

- **位置管理**：3D位置、速度、朝向
- **移动控制**：移动到目标位置、停止移动
- **路径系统**：路径节点、路径跟随
- **碰撞检测**：位置碰撞检查

```cpp
auto movement = character->getComponent<MovementComponent>();
movement->moveTo(Vector3(10.0f, 0.0f, 10.0f));
movement->setSpeed(5.0f);
movement->jump();
```

#### NetworkSyncComponent（网络同步组件）

处理网络同步和RPC：

- **状态同步**：位置、状态、属性同步
- **RPC系统**：远程过程调用
- **预测与回滚**：客户端预测、服务器校验
- **优先级管理**：不同优先级的同步频率

```cpp
auto network = character->getComponent<NetworkSyncComponent>();
network->setSyncPriority(SyncPriority::HIGH);
network->enablePrediction(true);
network->sendRPC("moveTo", parameters, target_id);
```

### CharacterManager（角色管理器）

单例模式，统一管理所有角色：

- **角色创建**：创建角色、玩家、NPC
- **角色查询**：按ID查询、按范围查询
- **角色管理**：移除角色、清理所有角色
- **批量更新**：统一更新所有角色

```cpp
CharacterManager& manager = CharacterManager::getInstance();
manager.initialize();

Player* player = manager.createPlayer(1001, "Hero", 10001);
NPC* npc = manager.createNPC(2001, "Goblin", NPCType::MONSTER);

manager.update(delta_time);
manager.removeCharacter(1001);

manager.shutdown();
```

## 构建说明

### 前置要求

- CMake 3.10+
- C++11兼容的编译器（GCC 4.8+, Clang 3.3+, MSVC 2015+）

### 构建步骤

```bash
cd character_system
mkdir build && cd build
cmake ..
cmake --build .
```

### 构建示例

```bash
cmake -DBUILD_EXAMPLES=ON ..
cmake --build .
```

### 运行示例

```bash
./bin/character_example
```

## 使用示例

### 创建角色并添加组件

```cpp
#include "character/character_manager.h"
#include "character/player.h"
#include "character/combat_component.h"
#include "character/movement_component.h"

using namespace game::character;

int main() {
    CharacterManager& manager = CharacterManager::getInstance();
    manager.initialize();
    
    // 创建玩家
    Player* player = manager.createPlayer(1001, "Hero", 10001);
    
    // 添加战斗组件
    auto combat = std::make_unique<CombatComponent>(player);
    combat->setOnDeathCallback([]() {
        std::cout << "Player died!" << std::endl;
    });
    player->addComponent(std::move(combat));
    
    // 添加移动组件
    auto movement = std::make_unique<MovementComponent>(player);
    movement->setOnMoveCallback([](const Vector3& pos) {
        std::cout << "Moved to: " << pos.x << ", " << pos.y << ", " << pos.z << std::endl;
    });
    player->addComponent(std::move(movement));
    
    // 初始化组件
    player->getComponent<CombatComponent>()->initialize();
    player->getComponent<MovementComponent>()->initialize();
    
    // 游戏循环
    while (true) {
        manager.update(0.1f);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    manager.shutdown();
    return 0;
}
```

### 战斗系统示例

```cpp
auto player_combat = player->getComponent<CombatComponent>();
auto npc_combat = npc->getComponent<CombatComponent>();

// 玩家攻击NPC
player_combat->attack(npc);

// NPC反击
npc_combat->attack(player);

// 检查生命值
std::cout << "Player HP: " << player_combat->getAttributes().health << std::endl;
std::cout << "NPC HP: " << npc_combat->getAttributes().health << std::endl;
```

### 移动系统示例

```cpp
auto movement = player->getComponent<MovementComponent>();

// 移动到目标位置
movement->moveTo(Vector3(10.0f, 0.0f, 10.0f));

// 设置移动速度
movement->setSpeed(5.0f);

// 跳跃
movement->jump();

// 停止移动
movement->stopMovement();
```

### 网络同步示例

```cpp
auto network = player->getComponent<NetworkSyncComponent>();

// 设置同步优先级
network->setSyncPriority(SyncPriority::HIGH);

// 启用预测
network->enablePrediction(true);

// 启用回滚
network->enableRollback(true);

// 发送RPC
std::vector<uint8_t> params;
network->sendRPC("moveTo", params, target_id);
```

## 设计模式

### ECS架构

- **Entity（实体）**：Character、Player、NPC
- **Component（组件）**：CombatComponent、MovementComponent、NetworkSyncComponent
- **System（系统）**：CharacterManager

### 单例模式

CharacterManager使用单例模式，确保全局只有一个实例。

### 组件模式

角色由多个组件组成，每个组件负责特定的功能。

### 回调模式

组件通过回调函数通知外部事件，如死亡、移动、同步等。

## 性能优化

- **对象池**：重用对象实例，减少内存分配
- **批量更新**：统一更新所有角色，减少函数调用开销
- **脏标记**：只同步变化的数据，减少网络流量
- **空间分区**：按范围查询时使用空间分区提高效率

## 扩展功能

### 添加新组件

1. 继承Component基类
2. 实现initialize()和update()方法
3. 在角色中添加组件

```cpp
class MyComponent : public Component {
public:
    MyComponent(Character* owner) : Component(owner) {}
    
    void initialize() override {
        // 初始化逻辑
    }
    
    void update(float delta_time) override {
        // 更新逻辑
    }
};

// 添加到角色
character->addComponent(std::make_unique<MyComponent>(character));
```

### 添加新角色类型

1. 继承Character基类
2. 添加特有的属性和方法
3. 在CharacterManager中添加创建方法

## 注意事项

1. **线程安全**：CharacterManager内部使用互斥锁保护共享数据
2. **内存管理**：使用智能指针管理组件生命周期
3. **性能考虑**：避免在update()中进行耗时操作
4. **网络同步**：合理设置同步间隔和优先级

## 已实现功能清单

### 核心系统
- [x] 基础角色类（Character）
- [x] 角色类型定义（CharacterType、CharacterState）
- [x] 组件基类（Component）
- [x] 角色管理器（CharacterManager）

### 战斗系统
- [x] 战斗组件（CombatComponent）
- [x] 战斗状态机（IDLE、MOVING、ATTACKING、CASTING、HIT、DEAD）
- [x] 技能系统（技能管理、冷却时间、施法判定）
- [x] 伤害计算（物理/魔法伤害、暴击、护甲穿透）
- [x] 仇恨系统（目标锁定、仇恨值管理、衰减）

### 移动系统
- [x] 移动组件（MovementComponent）
- [x] 3D位置管理（Vector3）
- [x] 移动控制（移动到目标、停止移动）
- [x] 路径系统（路径节点、路径跟随）
- [x] 跳跃功能
- [x] 碰撞检测框架

### 网络同步
- [x] 网络同步组件（NetworkSyncComponent）
- [x] 状态同步（位置、状态、属性）
- [x] RPC系统（远程过程调用）
- [x] 预测与回滚（客户端预测、服务器校验）
- [x] 优先级管理（LOW、MEDIUM、HIGH、CRITICAL）
- [x] 脏标记系统（只同步变化的数据）

### 玩家系统
- [x] 玩家类（Player）
- [x] 等级系统（经验值获取、升级处理）
- [x] 经济系统（金币、钻石管理）
- [x] 数据持久化接口（保存/加载玩家数据）
- [x] 在线状态管理（登录/登出时间记录）

### NPC系统
- [x] NPC类（NPC）
- [x] AI状态机（IDLE、PATROL、CHASE、ATTACK、FLEE、DEAD）
- [x] 巡逻系统（随机巡逻、路径跟随）
- [x] 追击系统（目标追踪、距离判断）
- [x] 攻击系统（自动攻击、攻击范围判断）
- [x] 逃跑系统（远离威胁、返回出生点）
- [x] 仇恨系统（目标锁定、仇恨值衰减）
- [x] 重生系统（死亡后自动重生）

### 角色管理
- [x] 单例模式实现
- [x] 角色创建（角色、玩家、NPC）
- [x] 角色查询（按ID查询、按范围查询）
- [x] 批量更新（统一更新所有角色）
- [x] 线程安全（互斥锁保护）
- [x] 回调系统（创建、移除事件通知）

## 待扩展功能

根据设计文档，以下功能可以后续添加：

- [ ] 装备系统组件（EquipmentComponent）
- [ ] 状态效果组件（StatusEffectComponent）
- [ ] 成就系统组件（AchievementComponent）
- [ ] 任务系统组件（QuestComponent）
- [ ] 道具系统组件（ItemComponent）
- [ ] 背包系统组件（InventoryComponent）
- [ ] 经济系统组件（EconomyComponent）
- [ ] 技能系统组件（SkillTreeComponent）
- [ ] 好友系统组件（FriendComponent）
- [ ] 社交系统组件（SocialComponent）
- [ ] 装备槽位系统
- [ ] 物品效果系统
- [ ] Buff/Debuff系统
- [ ] 持续效果系统（DoT、HoT）
- [ ] 效果堆叠规则
- [ ] AI行为树优化
- [ ] 导航网格集成
- [ ] 更多网络同步策略

## 许可证

本项目采用MIT许可证。

## 贡献

欢迎提交Issue和Pull Request！

---

**注意**：本项目仍在开发中，部分功能可能尚未完善。使用时请注意测试和验证。
