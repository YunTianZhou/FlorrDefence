# Florr Defence


**Florr Defence** is a tower-defence game inspired by the cards,
petals, enemies, and rarity progression of [florr.io](https://florr.io/). Build a
defence, getting powerful petals from shops, crafting, and talents, and survive increasingly
dangerous enemy waves.


> [!NOTE]
> The game is NOT under active development. The current configuration is still unbalanced.
> Unfortunately I do not have time to balance the configuration. I will be thankful if you are willing to help (see Contributing)!


## Screenshots


![Florr Defence gameplay](gameplay.png)


## Gameplay


Enemy mobs follow the path toward your flower. Place cards on compatible map tiles
to create attacking, defensive, summoning, or support towers. Defeated enemies
award coins and experience that can be invested into new cards and permanent
talent upgrades.


Current game include:

- Attacking, defensive, summoning, multishot, and buff towers
- 14 enemy types, including specialized enemies and bosses
- All 8 rarities for towers, enemies, and talents
- A rotating shop, card crafting, backpack, and a talent tree
- Automatic and manual JSON saves, including Save As and Open dialogs

## Controls


| Input | Action |
| --- | --- |
| Left click / drag | Select cards, interact with menus, and place or move towers |
| Right click a backpack card | Automatically place one card |
| Shift + right click a backpack card | Automatically place as many cards from that stack as possible |
| Right click a tower | Remove it and return its card to the backpack |
| Shift + right click a tower | Remove every matching tower |
| Shift + click a shop item | Buy the maximum affordable amount |
| Mouse wheel | Scroll the active backpack, shop, crafting, or talent panel |
| Ctrl + S | Save to the configured default path while alive |
| Ctrl + Shift + S | Save As while alive |
| Ctrl + O | Open a save file |
| Hold G | Show petal rarity |
| Hold H | Show enemy rarity |


## Building from source


### Requirements


- A C++20 compiler
- CMake 3.22 or newer on Linux, or CMake 3.24 or newer on Windows
- Git
- The platform development libraries required by SFML/OpenGL


### Windows


Run these commands from an x64 Native Tools command prompt with Ninja available:


```powershell
git clone --recurse-submodules https://github.com/YunTianZhou/FlorrDefence.git
cd FlorrDefence
cmake --preset x64-release
cmake --build out/build/x64-release
cd out/build/x64-release/FlorrDefence
./FlorrDefence.exe
```


The build copies the `res` directory beside the executable. Run the game from
that output directory because assets and configuration files are resolved from
the current working directory.


### Linux


> [!WARNING]
> This game may have a big performance issue on Linux, we recommand playing it on Windows.


```bash
git clone --recurse-submodules https://github.com/YunTianZhou/FlorrDefence.git
cd FlorrDefence
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
cd build/FlorrDefence
./FlorrDefence
```


## Configuration


On first launch, the game copies
[`settings_default.json`](FlorrDefence/res/config/settings_default.json) to
`settings.json` in the working directory.


| Setting | Default | Purpose |
| --- | ---: | --- |
| `load_path_default` | `FlorrDefence.json` | Save loaded when the game starts |
| `save_path_default` | `FlorrDefence.json` | Destination used by Ctrl + S and autosave |
| `auto_save_enabled` | `true` | Enables periodic saving |
| `auto_save_interval_seconds` | `60` | Time between autosaves |
| `vsync_enabled` | `true` | Requests display-synchronized rendering; a 60 FPS fallback is used when unavailable |
| `show_console` | `false` | Shows the console window on Windows |
| `debug_mode` | `false` | Enables FPS output and development shortcuts |


## Contributing

Currently the game configurations are still incomplete and unbalanced.

- Tower/Mob are missing certain rarities.
- Talent is not balanced.
- Mob spawning is not balanced. It either feels too boring or overwhelming.

You can help by:

- Reporting bugs
- Assisting with balancing configurations
- Improving game mechanics
- Any other way you can think of!


Bug reports and focused pull requests are welcome. 

When reporting a problem, include your operating system, compiler, build type, reproduction steps, and any
console output. Keep gameplay-data changes separate from engine changes where
possible so balancing differences are easy to review.

## Attribution

**Florr Defence is an independent fan project inspired by florr.io and is not
affiliated with its creators.** It uses
[SFML](https://www.sfml-dev.org/),
[nlohmann/json](https://github.com/nlohmann/json),
[portable-file-dialogs](https://github.com/samhocevar/portable-file-dialogs), and
[Cornered](https://github.com/metaquarx/Cornered).
