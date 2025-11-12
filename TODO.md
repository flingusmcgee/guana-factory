# Feature Roadmap & Tasks

How to use this file
- Mark completed tasks with `- [x]`.
- Edit assigned tasks or estimates as needed.
- Ask me to commit updates or open a PR when you're ready.

---

## High priority (ship-critical)

- [ ] Fix camera smoothing edge-cases — (Owner: ) — Priority: P0 — Est: 1d
  - Description: Camera jitter. Stabilize by flushing mouse delta and clamping large deltas.
  - Acceptance: No visible camera jump when unlocking/locking rapidly; unit manual test passes.

- [ ] Resolve remaining multi-platform filesystem/paths — (Owner: ) — Priority: P0 — Est: 1d
  - Description: Ensure consistent path handling for assets on Windows and Linux; unify config defaults.
  - Acceptance: No editor/tooling squiggles; `build.ps1` and manual Linux build both succeed.

---

## Medium priority (next milestone)

- [ ] Improve logging system (Owner: ) — Priority: P1 — Est: 1d
  - Tasks:
    - Add configurable log level (INFO/WARN/ERROR/DEBUG).
    - Option to route debug logs to separate file.
  - Acceptance: `config.ini` toggles debug-level logging without recompiling.

- [ ] Collision system improvements (Owner: ) — Priority: P1 — Est: 2d
  - Tasks:
    - Add continuous contact handling (onEnter/onStay/onExit events).
    - Add per-pair rate-limiting config.
  - Acceptance: onEnter fires once per contact, onStay fires at configurable rate, onExit fires when separated.

- [ ] Debug HUD polish (Owner: ) — Priority: P1 — Est: 0.5d
  - Tasks:
    - Add toggleable extra stats (draw calls, GPU mem usage if available).
    - Improve layout for smaller screens.
  - Acceptance: HUD toggle shows additional metrics and does not impact frame time significantly.

---

## Low priority / Nice-to-have

- [ ] Asset hot-reload (Owner: ) — Priority: P2 — Est: 3d
  - Description: Watch `res/` and reload changed textures/models at runtime.
  - Acceptance: Updating a texture file updates the in-game texture without restarting.

- [ ] In-game entity inspector (Owner: ) — Priority: P2 — Est: 2d
  - Description: Simple UI to select an entity and view/edit basic properties.
  - Acceptance: Can change position/color/model_id at runtime and see immediate effect.

---

## Content & design tasks

- [ ] Add new archetypes (cube variants) — Priority: P1 — Est: 0.5d
  - Create additional archetype files (different colors, physics settings).
  - Acceptance: New archetypes load and can be used via the spawn system.

- [ ] Tweak default camera sensitivity and controls — Priority: P1 — Est: 0.25d
  - Acceptance: Feels nice; update `config.ini` defaults.

---

## Housekeeping / repo maintenance

- [ ] Add CI build for Windows and Linux (Owner: ) — Priority: P1 — Est: 2d
  - Acceptance: GitHub Actions job that builds with MinGW and g++ on push and reports status.

- [ ] Add CONTRIBUTING.md and coding style guide (Owner: ) — Priority: P2 — Est: 0.5d
  - Acceptance: Adds a clear guide for PRs and commit message conventions.

---

## Quick fixes and tweaks (small, pick as time permits)

- [ ] Make collision logs debug-only (Owner: ) — Est: 0.25d
- [ ] Provide an option to change default `log.dir` in `config.ini` (Owner: ) — Est: 0.1d
- [ ] Replace remaining platform-specific includes with thin wrappers (Owner: ) — Est: 0.5d
