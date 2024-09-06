# WoW Raid Overlay [PATCHED AND ARCHIVED]

A fast raid overlay that abuses the "Advanced Network Logging" feature in World of Warcraft to display information that weakauras cannot. This will be patched before the next season (season 3 dragonflight)

## Features

- (almost) Real-time auras
- Movable components
- Raid leader mode (for when you need more information)
- Blazingly fast

## Current encounters

- Echo of Neltharion

## Get started

If you wish to use the application, follow the bellow steps:

1. Download from the releases tab (optionally compile it yourself)

2. Start the application to generate the `settings.json` file

`Note:` Most of the remaining steps can now be done inside the overlay under the settings window.   

3. Modify the `settings.json` with your desired settings. MAKE SURE to fill out the `player_name` field with your in-game character name (without realm. E.g OrcLover).

4. Enter the player list in the order you wish to use for your raid team. (For Echo of Neltharion)

5. Start the application, move it wherever you want and alt tab into the game. It will disappear but reappear when it is needed.

`Note:` if you wish to move it more, just alt-tab back into the overlay's window and it will reappear allowing you to move it again.

6. ???

7. Profit

## How to build

To build the project, follow these steps:

1. Clone the repository:
   ```bash
   git clone https://github.com/bonezone2001/raid-overlay.git
   ```

2. Install vcpkg if not already installed (optional)

3. Install / link GLFW and GLEW

4. Build using whatever build pipeline you use.

5. Launch the overlay executable and configure the settings as needed.

## Contributing

Contributions to the project are welcome! If you'd like to contribute, follow these steps:

1. Fork the repository.

2. Create a new branch for your feature or bug fix:
   ```bash
   git checkout -b feature/new-feature
   ```

3. Make your changes and commit them:
   ```bash
   git commit -m "Add new feature: XYZ"
   ```

4. Push your changes to your forked repository:
   ```bash
   git push origin feature/new-feature
   ```

5. Open a pull request on the main repository's `main` branch.

## License

This project is licensed under the [Apache 2.0 License](LICENSE).

## Contact

If you have any questions or suggestions, feel free to contact me

- [Kyle Pelham](https://github.com/bonezone2001)

Happy raiding!
