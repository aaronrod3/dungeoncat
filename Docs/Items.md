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

**Itemized weapon variety (post-beta, added 2026-08-19)**: each class's one locked weapon *type* above never changes — itemization varies stat rolls and appearance on that type, not the moveset. A Knight always wields a sword+shield; itemization is about *which* sword+shield, via the same affix system below. This keeps `Docs/Classes.md`'s ability-slot philosophy untouched — no itemized weapon ever changes which abilities a class has access to.

## Items / consumables (beta list)

- [ ] 2 stat trinkets — one +Armor, one +MoveSpeed. Auto-apply their `GameplayEffect` on pickup (no equip step, no inventory UI for the beta — keeps co-op pickup simple and avoids scope creep into a full inventory screen).
- [ ] 1 consumable — a heal-equivalent, usable directly from a simple carried-list.
- [ ] 1-2 currency/cosmetic-only drops.

Pickup UX: interact-prompt via `UDCInteractableComponent` (ported, P0-audit-confirmed reusable), not walk-over auto-pickup — keeps "who grabs it" server-authoritative-simple in co-op.

## Drop tables

One `DA_DC_LootTable_*` per enemy archetype (melee chaser, ranged spitter, swarm/leaper, Brute/Tank — `Docs/Bestiary.md`) + one per room type that can hold loot, rolled server-side only. Rarity tiers: Common 70% / Uncommon 20% / Rare 8% / Epic 2%.

**Boss loot tables** (distinct from the regular tables above, added 2026-08-19): every boss (`Docs/Bestiary.md`) rolls from its own `DA_DC_LootTable_Boss_*`, weighted much higher toward the top tiers — Common 10% / Uncommon 30% / Rare 40% / Epic 20% — and rolls **multiple times, not once**:
- **Guaranteed rolls = party size at the time of the kill** (1 for solo, 2 for the beta's 2-player co-op, scaling to 4 later) — every player gets a shot at boss loot without fighting over one drop, consistent with the existing interact-prompt "who grabs it stays server-authoritative-simple" pickup design.
- **Bonus roll chance on top**: a flat 25% chance at +1 extra roll beyond the guaranteed ones, so a boss kill still feels variably exciting rather than a flat, predictable "one per player" payout every time.
- Both numbers (`NumGuaranteedRolls`, `BonusRollChance`) live on the `DA_DC_LootTable_Boss_*` data asset per boss, not hardcoded — a harder or later-game boss can be tuned to roll more without new code. Technical note: `SystemsDesign.md` §6.

## Itemization system (post-beta, added 2026-08-19)

Deliberately small — a handful of stat affixes rather than a sprawling affix system, matching the beta's existing "4 tiers is enough to feel ARPG-ish" philosophy even as this extends past the beta.

**Affix pool** — 6 total, matching `UDCAttributeSet`'s existing fields one-to-one (no new stats invented just for itemization):

| Affix | Roll range |
|---|---|
| +MaxHealth | 5-15 |
| +MaxStamina | 5-15 |
| +Armor | 2-6 |
| +MoveSpeed | +2-5% |
| +CritChance | +2-5% |
| +CritMultiplier | +0.1-0.25x |

**Affix count by rarity** (drop-odds already fixed above, this adds affix count per tier):

| Rarity | Affix count |
|---|---|
| Common | 0 (a plain item — stat trinkets are Uncommon+ in practice) |
| Uncommon | 1 |
| Rare | 2 |
| Epic | 3 |

Weapons roll from the same 6-affix pool as trinkets — no separate weapon-only affix subsystem, keeps the system uniform. Technical shape (affix struct, DataTable, roll timing): `SystemsDesign.md` §6.

## Uniques (post-beta, added 2026-08-19)

One per class, a fixed (non-random) effect instead of rolled affixes — first-draft names/effects, not final:

| Class | Unique | Effect |
|---|---|---|
| Knight | Barnacle's Bite | Sword — cuts Headbutt's cooldown by a fixed amount |
| Rogue | Widow's Kiss | Daggers — Ambush's bonus damage always applies (no Slink/rear-cone requirement), at a smaller flat multiplier (~×1.5 instead of the conditional ×2) — trades ceiling for consistency |
| Wizard | Coven's Eye | Orb — Evil Eye's mark auto-spreads to a second enemy, same effect as the Coven spec fork but as an item rather than a Loadout choice |
| Healer | Kneading Paws | Gauntlets — Biscuits auto-triggers at the start of a fight, freeing the button to be used again once off cooldown |

## Cosmetics / customization (post-beta, added 2026-08-19)

Fur color/pattern variants + a small accessory set + a size slider, per `GameDevPlan.md` §4.3's modular character/customization plan:

- **Fur colors**: Tabby, Black, Orange, Calico, Grey, White — a small named-swatch set leaning into recognizable real-world cat coats for the "cat power fantasy" pillar.
- **Fur patterns**: Solid, Tabby-stripe, Patched/Calico, Spotted.
- **Accessories**: Collar (a few color variants), Bandana, a small Hat, and a Cape reserved for higher-rarity cosmetic prestige items.
- **Size**: a slider within bounds still TBD — depends on `AssetPipeline.md`'s flagged blend-shape-vs-bone-scaling fork, which decides how size is actually implemented before exact min/max numbers can be set.

## Skill-gated containers

Some containers require a leveled skill to open (see `Docs/Classes.md`'s "Skill & progression system" section for the full framework — Nimble Paws is the relevant universal skill). Technical hook: a lockable container carries `RequiredSkill` (FGameplayTag) + `RequiredSkillLevel` (int32); locked until the opening player's tracked skill level meets the requirement. What's actually *inside* a skill-gated container isn't decided yet — likely a small pool of especially good beta-scope loot (a rare trinket, extra currency), not a separate item tier of its own.
