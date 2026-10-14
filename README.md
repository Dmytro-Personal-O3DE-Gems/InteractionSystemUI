# Interaction System UI

An optional on-screen **interaction prompt** for the [Interaction System](https://github.com/Dmytro-Personal-O3DE-Gems/InteractionSystem) gem, built with LyShine.

When the player looks at an interactable, the prompt shows:

- **which actions are possible**: only press, only hold, or both;
- **the label of each action** ("Open", "Unlock", …), taken from the object itself and updated live;
- **a progress ring** that fills while a hold is in progress.

When there is no target, nothing is shown.

The core gem knows nothing about UI. It only exposes data through its buses, and this gem draws it. If you need a different look, you can use this gem as a reference or replace it entirely, without touching the core.

## Contents

- [Requirements](#requirements)
- [Installation](#installation)
- [What is inside](#what-is-inside)
- [Setup](#setup)
- [Component settings](#component-settings)
- [Canvas layout](#canvas-layout)
- [How it works](#how-it-works)
- [Customizing](#customizing)
- [Troubleshooting](#troubleshooting)
- [Status and roadmap](#status-and-roadmap)
- [License](#license)

## Requirements

| | |
|---|---|
| Engine | O3DE **26.05** (developed and tested against tag `2605.0`) |
| Gems | **InteractionSystem** and **LyShine** (both declared as dependencies in `gem.json`) |

The player must already be set up for interactions: **Interactor**, **Interactor Input**, and an **Interaction Overlap Finder** whose *Owner Entity* is the player. See the core gem's README.

## Installation

1. Clone the repository outside the engine, for example `D:\O3DE\UserGems\InteractionSystemUI`.
2. Register it:
   ```
   o3de register --gem-path D:\O3DE\UserGems\InteractionSystemUI
   ```
3. Enable **Interaction System UI** in your project: Project Manager → *Configure Gems*, or add `"InteractionSystemUI"` to `gem_names` in `project.json`. **InteractionSystem** and **LyShine** must be enabled too.
4. Rebuild the project **with the Editor closed**. Otherwise the linker cannot overwrite the gem's DLL.

## What is inside

| Path | What |
|---|---|
| `Code/Source/InteractionPromptComponent.*` | the **Interaction Prompt** component |
| `Assets/UICanvas/InteractionCanvas.uicanvas` | a ready canvas with the three prompt rows and the progress rings |
| `Assets/UI/Textures/ProgressRing.png` | 256×256 white ring with a transparent centre: tinted by the Image's Color, filled radially |

## Setup

All three components go on the **player's root entity**, the same entity the Interactor and the finder's *Owner Entity* point to:

1. **UI Canvas Asset Ref** (LyShine)
   - *Canvas pathname*: `uicanvas/interactioncanvas.uicanvas` (or your own canvas);
   - **Load automatically: ON.** It is **off by default**. With it off the canvas is never loaded and the prompt stays empty. The log shows a warning about this.
2. **Interaction Prompt** (category *Interaction*). The element names in its settings already match the bundled canvas.
3. *(already there from the core gem)* **Interactor** and **Interactor Input**.

Enter game mode and look at an interactable.

The prompt requires *UI Canvas Asset Ref* on the same entity: without it the editor marks the component as missing a required component. It does **not** require the finder, because the finder usually sits on the camera, another entity. Component requirements only work within one entity.

## Component settings

**Element names.** Names of canvas elements, exactly as in the UI Editor hierarchy:

| Setting | Default | Searched in |
|---|---|---|
| Press Row | `PressRow` | the whole canvas |
| Hold Row | `HoldRow` | the whole canvas |
| Press Or Hold Row | `PressOrHoldRow` | the whole canvas |
| Press Row Label | `PressLabel` | inside *Press Row* |
| Hold Row Label | `HoldLabel` | inside *Hold Row* |
| Press Or Hold: Press Label | `OrPressLabel` | inside *Press Or Hold Row* |
| Press Or Hold: Hold Label | `OnHoldLabel` | inside *Press Or Hold Row* |
| Ring Fill | `RingFill` | inside *Hold Row* **and** inside *Press Or Hold Row* |

- **Rows** are searched in the whole canvas and the first match is used, so row names must be **unique on the canvas**.
- **Labels and rings** are searched only inside their row, so their names only need to be unique **within the row**. The same `RingFill` name works for both rings.

**Ring Decay Speed** (1/s, default **10**, `0` = empty at once) — how fast the ring drains when a hold is released early. After `t` seconds, `e^(−speed·t)` of the fill is left:

| After | 0.1 s | 0.2 s | 0.3 s | 0.5 s |
|---|---|---|---|---|
| Left at speed 10 | 37 % | 14 % | 5 % | 0.7 % |

## Canvas layout

The bundled canvas has one row per interaction mode. Exactly one row is visible at a time, or none:

```
PressRow                 shown for "Press" objects
├── PressKeyHint         "[E]"
└── PressLabel           label of the press action
HoldRow                  shown for "Hold" objects
├── RingBackground       ring behind the key hint
├── RingFill             hold progress, drawn on top of the background
├── HoldKeyHint          "[E]", centred inside the ring
└── HoldLabel            label of the hold action
PressOrHoldRow           shown for "Press or hold" objects
├── OrPressRow
│   ├── OrKeyHint
│   └── OrPressLabel     label of the press action
└── OnHoldRow
    ├── RingBackground
    ├── RingFill
    ├── HoldKeyHint
    └── OnHoldLabel      label of the hold action
```

Only the three top-level rows, the four labels and `RingFill` are looked up by the component. The other elements (key hints, ring backgrounds, the inner rows) are pure layout, and their names do not matter to the code.

**Why a separate row per mode** instead of one row with parts hidden: a LyShine **Layout Row** keeps space for its disabled children. Hiding "Hold" inside a shared row leaves a gap where it was.

**Ring images** — two Images of the same size at the same spot:

| | `RingBackground` | `RingFill` |
|---|---|---|
| Sprite | `ProgressRing.png` | `ProgressRing.png` |
| Image Type | Stretched | Stretched |
| Fill Type | None | **Radial** |
| Color | grey, alpha ≈ 0.5 | white |
| Order | above in the hierarchy | below, so it is drawn on top |

- **Fill Type** is a separate setting from *Image Type*. With *None*, the fill amount has no effect.
- Radial fill starts at *Radial Fill Start Angle*, in degrees clockwise **from the top**. The direction is set by *Fill Clockwise*.
- If a row is a **Layout Row**, do not make the rings its direct children: the layout would treat them as cells. Put them inside the key hint element or another container.

## How it works

**Showing the right row.** The component listens to `InteractionOverlapFinderNotificationBus::OnTargetChanged` on its own entity. On each change it reads `InteractionInfo::m_mode` of the new target and enables the matching row. When the component activates, it also **pulls** the current target once with `GetCurrentTarget()`. Without that, a player who spawns already facing a door would see no prompt until the target changes, because no event is sent for a target that was already there.

**Labels.** While there is a target, every frame it asks `InteractableRequests::GetInteractableLabel(player, Press / Hold)`.
- Polling catches label changes the object cannot announce. For example, the player picked up a key and "Open" becomes "Unlock".
- The text is written only when it changes.
- Without a target the component is not ticking at all.

**Progress ring.** While there is a target, every frame it reads `InteractorRequests::GetHoldProgress()`:

| Situation | Ring |
|---|---|
| Holding | follows the real progress **immediately** |
| Released early | **drains smoothly** (*Ring Decay Speed*), frame-rate independent |
| Interaction done / denied (`OnInteractionDone` / `OnInteractionDenied`) | **empty at once** |
| Target changed or lost | **empty at once** |

Both rows get the same text and fill, even the hidden one, so switching rows never shows stale values.

## Customizing

- **Your own canvas.** Keep the three rows and put your own design inside them. Point *UI Canvas Asset Ref* to the new canvas, and type its element names into the component if they differ.
- **Colors and sizes.** Edit the canvas in the UI Editor: no code involved. The ring texture is white on purpose, so the *Color* of each Image decides its color.
- **Labels.** They come from the object, not from this gem. Set default labels in the object's *Interactable → Interaction Info*. Override them at runtime with `ReturnInteractableLabel` from C++ or Script Canvas. See the core gem's README.
- **Do not drive the same canvas elements from Script Canvas** while this component is active. Both would fight over them.

## Troubleshooting

All warnings are written to `<Project>/user/log/Editor.log` with the prefix `(InteractionPrompt)`.

| Symptom | Cause | Fix |
|---|---|---|
| Nothing appears; log: *"has no loaded UI canvas"* | *Load automatically* is off on *UI Canvas Asset Ref* | turn it on |
| Log: *"UI element 'X' was not found on the canvas"* / *"…inside its row"* | the name in the component differs from the UI Editor name | fix the name; row names must be unique on the canvas |
| A prompt is shown, but always the wrong row | two elements with the same row name | rename one of them |
| The ring never fills | *Fill Type* of `RingFill` is *None* | set it to *Radial* |
| The ring starts not at the top | *Radial Fill Start Angle* | set 0 (top), check *Fill Clockwise* |
| The ring sits in a weird place or is stretched | it is a direct child of a Layout Row | move it inside the key hint or a container |
| No prompt at all, no warnings | the finder's *Owner Entity* is not the entity with this component, or the target has no Interactable | check the core gem setup |
| Code changes have no effect | the DLL was rebuilt while the Editor was running | close the Editor and rebuild |

## Status and roadmap

Tested in the editor with a player character: all three modes, labels (default and overridden from Script Canvas), live label updates, the progress ring (fill, smooth drain, instant reset).

Not done yet:
- **Key hint is static text** (`[E]` in the canvas). The engine's input bindings offer no API to read which key an action is bound to, so key rebinding and gamepad glyphs need a separate input-hint system.
- **Display name** of the object (`InteractionInfo::m_displayName`) is not shown yet.
- **Localization**: labels are plain strings for now and will become localization keys.

## License

[MIT](LICENSE) © 2026 Dmytro Shyshkin
