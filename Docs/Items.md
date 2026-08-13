# Items

Living reference for the actual item/weapon/clothing/loot catalog — the creative/content "what." For technical architecture (the `UDCItemConfig` DataAsset shape, drop-table rolling, pickup flow), see `SystemsDesign.md` §6.

Ongoing doc — start here, keep adding as the beta item list grows and post-beta cosmetics/gear get designed.

---

## Weapons

One set per class (full identity/reasoning in `Docs/Classes.md`):

| Class | Weapon | Sockets |
|---|---|---|
| Knight | Sword + shield | `Socket.WeaponHandR` (sword), `Socket.WeaponHandL` (shield) |
| Rogue | Dual daggers | `Socket.WeaponHandR` + `Socket.WeaponHandL` (both offense — generalizes the `WeaponHandL` socket beyond "shield only," see `AssetPipeline.md` §3) |
| Wizard | Hex-focus staff or orb | `Socket.WeaponHandR` |
| Healer | Claw-gauntlets or light mace | `Socket.WeaponHandR` |

Only Knight's weapon is needed for the beta. The other three are post-beta.

## Items / consumables (beta list)

- [ ] 2 stat trinkets — one +Armor, one +MoveSpeed. Auto-apply their `GameplayEffect` on pickup (no equip step, no inventory UI for the beta — keeps co-op pickup simple and avoids scope creep into a full inventory screen).
- [ ] 1 consumable — a heal-equivalent, usable directly from a simple carried-list.
- [ ] 1-2 currency/cosmetic-only drops.

Pickup UX: interact-prompt via `UDCInteractableComponent` (ported, P0-audit-confirmed reusable), not walk-over auto-pickup — keeps "who grabs it" server-authoritative-simple in co-op.

## Drop tables

One `DA_DC_LootTable_*` per enemy archetype (melee chaser, ranged spitter, swarm/leaper) + one per room type that can hold loot, rolled server-side only. Rarity tiers: Common 70% / Uncommon 20% / Rare 8% / Epic 2%.

## Clothing / cosmetics (post-beta — content-scope table already marks this "Placeholder only" for the beta)

Fur color/pattern variants + a small set of modular head/accessory meshes (`GameDevPlan.md` §4.3's modular character/customization plan). Tracked here for when that pass starts; no content decided yet.

## Skill-gated containers

Some containers require a leveled skill to open (see `Docs/Classes.md`'s "Skill & progression system" section for the full framework — Nimble Paws is the relevant universal skill). Technical hook: a lockable container carries `RequiredSkill` (FGameplayTag) + `RequiredSkillLevel` (int32); locked until the opening player's tracked skill level meets the requirement. What's actually *inside* a skill-gated container isn't decided yet — likely a small pool of especially good beta-scope loot (a rare trinket, extra currency), not a separate item tier of its own.
