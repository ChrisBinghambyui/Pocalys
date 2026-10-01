#include "MetaSave.h"
#include "SaveFile.h"
#include "StashData.h"
#include "Item.h"
#include "ItemData.h"
#include "WeaponData.h"
#include "MaterialData.h"
#include "ArrowData.h"
#include "RosterData.h"
#include "FeatData.h"
#include "AbilityData.h"
#include "SkillData.h"
#include <string>
#include <vector>

static const char* SAVE_FILE_PATH = "savegame.dat";

// Item fields, in order: archetypeId, weaponTypeId, materialTier, condition, quantity, ammoTypeId.
// Ids are table indices, so the append-only rule for G_WEAPON_TYPES and G_MATERIAL_TIERS matters
// even more now: reordering them would silently change what a saved item is.
static void WriteItemFields(SaveWriter& writer, const Item& item)
{
    writer.WriteString(item.archetypeId);
    writer.WriteInt(item.weaponTypeId);
    writer.WriteInt(item.materialTier);
    writer.WriteInt((int)item.condition);
    writer.WriteInt(item.quantity);
    writer.WriteInt(item.ammoTypeId);
}

// False if the record is damaged or refers to content that no longer exists. That item is dropped, the load goes on.
static bool ReadItemFields(SaveReader& reader, Item& outItem)
{
    Item item;
    item.archetypeId = reader.ReadString();
    item.weaponTypeId = reader.ReadInt();
    item.materialTier = reader.ReadInt();
    int condition = reader.ReadInt();
    item.quantity = reader.ReadInt();
    item.ammoTypeId = reader.ReadInt();

    if (item.weaponTypeId < -1 || item.weaponTypeId >= (int)G_WEAPON_TYPES.size())
    {
        return false;
    }
    if (item.materialTier < -1 || item.materialTier >= (int)G_MATERIAL_TIERS.size())
    {
        return false;
    }
    if (condition < (int)CONDITION_PRISTINE || condition >(int)CONDITION_BROKEN)
    {
        return false;
    }
    item.condition = (ConditionTier)condition;
    if (item.quantity < 1)
    {
        return false;
    }
    if (item.ammoTypeId < -1)
    {
        return false;
    }
    if (item.ammoTypeId >= 0 && FindArrowType(item.ammoTypeId) == nullptr)
    {
        return false;
    }
    if (!item.archetypeId.empty())
    {
        bool found = false;
        for (size_t i = 0; i < G_ITEM_ARCHETYPES.size(); i++)
        {
            if (G_ITEM_ARCHETYPES[i].id == item.archetypeId)
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            return false;
        }
    }
    if (item.IsEmpty())
    {
        return false;
    }

    outItem = item;
    return true;
}
static void WriteIntListRecord(SaveWriter& writer, const std::string& tag, const std::vector<int>& values)
{
    writer.BeginRecord(tag);
    writer.WriteInt((int)values.size());
    for (size_t i = 0; i < values.size(); i++)
    {
        writer.WriteInt(values[i]);
    }
    writer.EndRecord();
}

static std::vector<int> ReadIntList(SaveReader& reader)
{
    std::vector<int> values;
    int count = reader.ReadInt();
    if (count > 1000)
    {
        count = 1000; // A damaged count should not run away
    }
    for (int i = 0; i < count; i++)
    {
        values.push_back(reader.ReadInt());
    }
    return values;
}

// kind: 0 = inventory, 1 = equipped slot (slot is the EquipSlot index), 2 = amulet, 3 = ring
static void WriteCharacterItem(SaveWriter& writer, int kind, int slot, const Item& item)
{
    writer.BeginRecord("charitem");
    writer.WriteInt(kind);
    writer.WriteInt(slot);
    WriteItemFields(writer, item);
    writer.EndRecord();
}

static void WriteCharacterRecords(SaveWriter& writer, const Player& player)
{
    writer.BeginRecord("char");
    writer.WriteString(player.name);
    writer.WriteString(player.className);
    writer.WriteString(player.birthsign);
    writer.WriteString(player.raceId);
    writer.WriteInt(player.level);
    writer.WriteInt(player.levelProgress);
    writer.WriteInt(player.unspentFeatPoints);
    writer.WriteInt(player.unspentBoonPoints);
    writer.WriteInt(player.hp);
    writer.WriteInt(player.maxHp);
    writer.WriteInt(player.stamina);
    writer.WriteInt(player.maxStamina);
    writer.WriteInt(player.mana);
    writer.WriteInt(player.maxMana);
    writer.WriteInt(player.str);
    writer.WriteInt(player.end);
    writer.WriteInt(player.agi);
    writer.WriteInt(player.intel);
    writer.WriteInt(player.wil);
    writer.WriteInt(player.per);
    writer.WriteInt(player.lck);
    writer.EndRecord();

    writer.BeginRecord("charskills");
    writer.WriteInt((int)player.skills.size());
    for (size_t i = 0; i < player.skills.size(); i++)
    {
        writer.WriteInt(player.skills[i].level);
        writer.WriteInt(player.skills[i].xp);
    }
    writer.EndRecord();

    WriteIntListRecord(writer, "charmajor", player.majorSkills);
    WriteIntListRecord(writer, "charminor", player.minorSkills);
    WriteIntListRecord(writer, "charfeats", player.activeFeats);
    WriteIntListRecord(writer, "charboons", player.activeBoons);

    writer.BeginRecord("charhotbar");
    writer.WriteInt((int)player.hotbar.size());
    for (size_t i = 0; i < player.hotbar.size(); i++)
    {
        writer.WriteString(player.hotbar[i]);
    }
    writer.EndRecord();

    for (size_t i = 0; i < player.inventory.size(); i++)
    {
        WriteCharacterItem(writer, 0, 0, player.inventory[i]);
    }
    for (int s = 0; s < SLOT_SINGLE_COUNT; s++)
    {
        if (!player.equippedSlots[s].IsEmpty())
        {
            WriteCharacterItem(writer, 1, s, player.equippedSlots[s]);
        }
    }
    for (size_t i = 0; i < player.equippedAmulets.size(); i++)
    {
        WriteCharacterItem(writer, 2, 0, player.equippedAmulets[i]);
    }
    for (size_t i = 0; i < player.equippedRings.size(); i++)
    {
        WriteCharacterItem(writer, 3, 0, player.equippedRings[i]);
    }
}

// Handles every record tag that belongs to a character. A "char" record starts a new roster entry and
// the records after it fill in the newest one. Unknown tags, and records with no "char" before them, are skipped.
static void ApplyCharacterRecord(SaveReader& reader, std::vector<Player>& roster)
{
    const std::string& tag = reader.GetTag();

    if (tag == "char")
    {
        Player fresh = Player();
        fresh.name = reader.ReadString();
        fresh.className = reader.ReadString();
        fresh.birthsign = reader.ReadString();
        fresh.raceId = reader.ReadString();
        fresh.level = reader.ReadInt();
        fresh.levelProgress = reader.ReadInt();
        fresh.unspentFeatPoints = reader.ReadInt();
        fresh.unspentBoonPoints = reader.ReadInt();
        fresh.hp = reader.ReadInt();
        fresh.maxHp = reader.ReadInt();
        fresh.stamina = reader.ReadInt();
        fresh.maxStamina = reader.ReadInt();
        fresh.mana = reader.ReadInt();
        fresh.maxMana = reader.ReadInt();
        fresh.str = reader.ReadInt();
        fresh.end = reader.ReadInt();
        fresh.agi = reader.ReadInt();
        fresh.intel = reader.ReadInt();
        fresh.wil = reader.ReadInt();
        fresh.per = reader.ReadInt();
        fresh.lck = reader.ReadInt();
        fresh.initSkills(); // Sized to the current skill table, the saved values are laid over it below
        roster.push_back(fresh);
        return;
    }

    if (roster.empty())
    {
        return;
    }
    Player& player = roster.back();

    if (tag == "charskills")
    {
        int count = reader.ReadInt();
        for (int i = 0; i < count; i++)
        {
            int level = reader.ReadInt();
            int xp = reader.ReadInt();
            if (i < (int)player.skills.size())
            {
                player.skills[i].level = level;
                player.skills[i].xp = xp;
            }
        }
    }
    else if (tag == "charmajor" || tag == "charminor")
    {
        std::vector<int> ids = ReadIntList(reader);
        std::vector<int> valid;
        for (size_t i = 0; i < ids.size(); i++)
        {
            if (ids[i] >= 0 && ids[i] < (int)G_SKILL_TYPES.size())
            {
                valid.push_back(ids[i]);
            }
        }
        if (tag == "charmajor")
        {
            player.majorSkills = valid;
        }
        else
        {
            player.minorSkills = valid;
        }
    }
    else if (tag == "charfeats" || tag == "charboons")
    {
        std::vector<int> ids = ReadIntList(reader);
        for (size_t i = 0; i < ids.size(); i++)
        {
            if (FindFeat(ids[i]) == nullptr)
            {
                continue; // Feat was removed from the table since this save
            }
            if (tag == "charfeats")
            {
                player.activeFeats.push_back(ids[i]);
            }
            else
            {
                player.activeBoons.push_back(ids[i]);
            }
        }
    }
    else if (tag == "charhotbar")
    {
        int count = reader.ReadInt();
        for (int i = 0; i < count; i++)
        {
            std::string abilityId = reader.ReadString();
            if (i < (int)player.hotbar.size() && FindAbility(abilityId) != nullptr)
            {
                player.hotbar[i] = abilityId;
            }
        }
    }
    else if (tag == "charitem")
    {
        int kind = reader.ReadInt();
        int slot = reader.ReadInt();
        Item item;
        if (!ReadItemFields(reader, item))
        {
            return;
        }
        if (kind == 0)
        {
            player.inventory.push_back(item);
        }
        else if (kind == 1)
        {
            if (slot >= 0 && slot < SLOT_SINGLE_COUNT)
            {
                player.equippedSlots[slot] = item;
            }
        }
        else if (kind == 2)
        {
            player.equippedAmulets.push_back(item);
        }
        else if (kind == 3)
        {
            player.equippedRings.push_back(item);
        }
    }
}

bool SaveMetaGame()
{
    SaveWriter writer;

    writer.BeginRecord("scrolls");
    writer.WriteInt(SAVE_FORMAT_VERSION);
    writer.EndRecord();

    for (size_t i = 0; i < G_STASH.items.size(); i++)
    {
        writer.BeginRecord("stashitem");
        WriteItemFields(writer, G_STASH.items[i]);
        writer.EndRecord();
    }

    for (size_t i = 0; i < G_ROSTER.size(); i++)
    {
        WriteCharacterRecords(writer, G_ROSTER[i]);
    }

    return writer.WriteToFile(SAVE_FILE_PATH);
}

bool LoadMetaGame()
{
    SaveReader reader;
    if (!reader.LoadFromFile(SAVE_FILE_PATH))
    {
        return false; // First launch, nothing saved yet
    }

    if (!reader.NextRecord() || reader.GetTag() != "scrolls")
    {
        return false;
    }
    int version = reader.ReadInt();
    if (version != SAVE_FORMAT_VERSION)
    {
        return false;
    }

    std::vector<Item> loadedStash;
    std::vector<Player> loadedRoster;
    while (reader.NextRecord())
    {
        if (reader.GetTag() == "stashitem")
        {
            Item item;
            if (ReadItemFields(reader, item))
            {
                loadedStash.push_back(item);
            }
        }
        else
        {
            ApplyCharacterRecord(reader, loadedRoster); // Skips tags it does not know, so newer files still load
        }
    }

    G_STASH.items = loadedStash;
    G_ROSTER = loadedRoster;
    return true;
}