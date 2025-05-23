#pragma once

#include <vector>
#include <memory>
#include <string_view>
#include <string>

#include "game/constants.h"

class Item;
class World;
class GameScript;
class Npc;
class Interactive;
class Serialize;

class Inventory final {
  public:
    Inventory();
    Inventory(Inventory&&)=default;
    Inventory& operator = (Inventory&&)=default;
    ~Inventory();

    bool         isEmpty() const;

    void         load(Serialize& s, Npc& owner);
    void         load(Serialize& s, Interactive& owner, World &w);
    void         save(Serialize& s) const;

    enum IteratorType : uint8_t {
      T_Inventory,
      T_Trade,
      T_Ransack,
      };

    class Iterator {
      public:
        const Item& operator*   () const;
        const Item* operator -> () const;

        size_t      count() const;
        bool        isEquipped() const;
        uint8_t     slot() const;

        Iterator&   operator++();

        bool        isValid() const;

      private:
        Iterator(IteratorType t, const Inventory* owner);
        void skipHidden();

        IteratorType     type  = T_Inventory;
        const Inventory* owner = nullptr;
        size_t           at    = 0;
        size_t           subId = 0;
      friend class Inventory;
      };

    Iterator     iterator(IteratorType t) const;

    int32_t      priceOf(size_t item) const;
    int32_t      sellPriceOf(size_t item) const;
    size_t       goldCount() const;
    size_t       itemCount(const size_t id) const;

    static void  transfer(Inventory& to, Inventory& from, Npc *fromNpc, size_t cls, size_t count, World &wrld);

    Item*  getItem(size_t instance);
    Item*  addItem(std::unique_ptr<Item>&& p);
    Item*  addItem(std::string_view name, size_t count, World &owner);
    Item*  addItem(size_t cls, size_t count, World &owner);
    void   delItem(size_t cls, size_t count, Npc &owner);
    bool   use    (size_t cls, Npc &owner, uint8_t slotHint, bool force);
    bool   equip  (size_t cls, Npc &owner, bool force);
    bool   unequip(size_t cls, Npc &owner);
    void   unequip(Item*  cls, Npc &owner);
    void   invalidateCond(Npc &owner);
    void   autoEquipWeapons(Npc &owner);
    void   equipArmor           (int32_t cls, Npc &owner);
    void   equipBestArmor       (Npc &owner);
    void   equipBestMeleeWeapon (Npc &owner);
    void   equipBestRangedWeapon(Npc &owner);
    void   unequipWeapons(GameScript &vm, Npc &owner);
    void   unequipArmor(GameScript &vm, Npc &owner);
    void   clear(GameScript &vm, Npc &owner, bool includeMissionItm = false);
    void   clear(GameScript &vm, Interactive &owner, bool includeMissionItm = false);
    bool   hasSpell(int32_t spl) const;
    bool   hasMissionItems() const;
    bool   hasRangedWeaponWithAmmo() const;

    void   updateArmorView (Npc& owner);
    void   updateSwordView (Npc& owner);
    void   updateBowView   (Npc& owner);
    void   updateShieldView(Npc& owner);
    void   updateRuneView  (Npc& owner);
    void   updateView      (Npc& owner);

    const Item*  activeWeapon() const;
    Item*  activeWeapon();
    void   switchActiveWeaponFist();
    void   switchActiveWeapon(Npc &owner, uint8_t slot);
    void   switchActiveSpell (int32_t spell, Npc &owner);

    Item*  currentArmor()          { return armor;      }
    Item*  currentMeleeWeapon()    { return melee;      }
    Item*  currentRangedWeapon()   { return range;      }
    Item*  currentSpell(uint8_t s) { return numslot[s]; }
    const Item*  currentSpell(uint8_t s) const { return numslot[s]; }
    Item*  findByFlags(ItmFlags f, uint32_t num) const;

    uint8_t currentSpellSlot() const;
    bool    hasStateItem() const;

    void   putCurrentToSlot(Npc& owner, std::string_view slot);
    void   putToSlot       (Npc& owner, size_t cls, std::string_view slot);
    bool   clearSlot       (Npc& owner, std::string_view slot, bool remove);
    void   putAmmunition   (Npc& owner, size_t cls, std::string_view slot);
    bool   putState        (Npc& owner, size_t cls, int state);

    static void moveItem(Npc& owner, Inventory& invNpc, Interactive& mobsi);

    void   setCurrentItem(size_t cls);
    void   setStateItem  (size_t cls);

  private:
    struct MdlSlot final {
      std::string slot;
      Item*       item = nullptr;
      };

    void   implLoad(Npc *owner, World &world, Serialize& s);
    void   implPutState(Npc& owner, size_t cls, std::string_view slot);

    bool   setSlot     (Item*& slot, Item *next, Npc &owner, bool force);
    bool   equipNumSlot(Item *next, uint8_t slotHint, Npc &owner, bool force);
    void   applyArmor  (Item& it, Npc &owner, int32_t sgn);

    Item*  findByClass(size_t cls);
    void   delItem    (Item* it, size_t count, Npc& owner);
    void   invalidateCond(Item*& slot,  Npc &owner);

    Item*  bestItem       (Npc &owner, ItmFlags f);
    Item*  bestArmor      (Npc &owner);
    Item*  bestMeleeWeapon(Npc &owner);
    Item*  bestRangedWeapon(Npc &owner);

    void   applyWeaponStats(Npc &owner, const Item& weap, int sgn);

    void        sortItems() const;
    static bool less   (const Item &il, const Item &ir);
    static int  orderId(const Item& l);
    uint8_t     slotId (Item*& slt) const;

    mutable std::vector<std::unique_ptr<Item>> items;
    mutable bool                               sorted=false;

    uint32_t                           indexOf(const Item* it) const;
    Item*                              readPtr(Serialize& fin);

    Item*                              armor      = nullptr;
    Item*                              belt       = nullptr;
    Item*                              amulet     = nullptr;
    Item*                              ringL      = nullptr;
    Item*                              ringR      = nullptr;

    Item**                             active     = nullptr;
    Item*                              melee      = nullptr;
    Item*                              range      = nullptr;
    Item*                              shield     = nullptr;
    Item*                              numslot[8] = {};
    std::vector<MdlSlot>               mdlSlots;
    MdlSlot                            ammotSlot, stateSlot;
    int32_t                            curItem=0;
    int32_t                            stateItem=0;
  };
