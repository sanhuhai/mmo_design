#ifndef COMPONENT_H
#define COMPONENT_H

#include <cstdint>
#include <typeinfo>
#include <string>
#include <functional>
#include <memory>
#include <unordered_map>

namespace game {
namespace character {

class Character;

enum class ComponentState {
    UNINITIALIZED,
    INITIALIZING,
    INITIALIZED,
    ACTIVE,
    PAUSED,
    DESTROYING,
    DESTROYED
};

enum class ComponentPriority {
    LOW = 0,
    NORMAL = 1,
    HIGH = 2,
    CRITICAL = 3
};

struct ComponentEvent {
    std::string event_name;
    void* data;
    uint64_t timestamp;
};

class Component {
public:
    Component(Character* owner);
    virtual ~Component();

    virtual void initialize();
    virtual void update(float delta_time);
    virtual void destroy();
    
    virtual void onEnable();
    virtual void onDisable();
    virtual void onPause();
    virtual void onResume();

    void setEnabled(bool enabled);
    bool isEnabled() const { return enabled_; }
    
    void setPaused(bool paused);
    bool isPaused() const { return paused_; }
    
    void setPriority(ComponentPriority priority);
    ComponentPriority getPriority() const { return priority_; }
    
    ComponentState getState() const { return state_; }
    
    const std::type_info& getTypeInfo() const { return typeid(*this); }
    std::string getTypeName() const { return std::string(getTypeInfo().name()); }
    
    Character* getOwner() const { return owner_; }
    void setOwner(Character* owner) { owner_ = owner; }
    
    uint64_t getComponentId() const { return component_id_; }
    
    void addEventListener(const std::string& event_name, std::function<void(const ComponentEvent&)> callback);
    void removeEventListener(const std::string& event_name);
    void triggerEvent(const std::string& event_name, void* data = nullptr);
    
    void setDependency(const std::string& component_type);
    bool hasDependency(const std::string& component_type) const;
    const std::vector<std::string>& getDependencies() const { return dependencies_; }
    
    virtual std::string serialize() const;
    virtual bool deserialize(const std::string& data);
    
    virtual Component* clone(Character* new_owner) const;
    
    virtual void debugPrint() const;
    
    bool isDirty() const { return is_dirty_; }
    void setDirty(bool dirty) { is_dirty_ = dirty; }
    
    void setCustomData(const std::string& key, void* data);
    void* getCustomData(const std::string& key) const;
    void removeCustomData(const std::string& key);

protected:
    Character* owner_;
    uint64_t component_id_;
    
    ComponentState state_;
    bool enabled_;
    bool paused_;
    ComponentPriority priority_;
    bool is_dirty_;
    
    std::unordered_map<std::string, std::function<void(const ComponentEvent&)>> event_listeners_;
    std::vector<std::string> dependencies_;
    std::unordered_map<std::string, void*> custom_data_;
    
    static uint64_t next_component_id_;
};

} 
} 

#endif 
