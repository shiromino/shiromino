# Running shiromino
You can run the game by just launching the executable. If your executable resides in a build tree, check [this section](BUILDING.md#running) for additional help.
<a name="custom-configuration-file"></a>
If you would like to specify a custom configuration file, you can do so with the command-line options `--configuration-file` or `-c`. For example, you could provide the flag like this from the source directory:
```shell
$ ./build/shiromino --configuration-file ./shiromino.ini
```
<a name="path-resolution"></a>
## Path resolution
Besides the game executable, the game needs to resolve the following files or paths in order to run properly.
### `shiromino.ini`
This configuration file is where you specify key bindings and other settings.

If no configuration file is specified, the game will look for this file here (in order):

1. `<executable directory>/shiromino.ini`
2. `<executable directory>/../etc/shiromino.ini`
3. `<current working directory>/shiromino.ini`
4. `$XDG_CONFIG_HOME/shiromino.ini`
5. `$HOME/.config/shiromino.ini`
6. `/usr/local/etc/shiromino.ini`
7. `/etc/shiromino.ini`

The first path that matches will be honored.

If none match, the game will try to create a default configuration file here (in order):

1. `$XDG_CONFIG_HOME/shiromino.ini`
2. `$HOME/.config/shiromino.ini`
3. `<executable directory>/shiromino.ini`

You can also [specify a custom configuration file](BUILDING.md#custom-configuration-file).
### `SHARE_PATH`
`SHARE_PATH` is a path to a directory which contains static game data such as assets or the license. The game assumes that there is a directory within `SHARE_PATH` named `data`.

If `SHARE_PATH` is left unspecified, the game will look for it here (in order):

1. `<executable directory>`
2. `<executable directory>/../share/shiromino`
3. `<current working directory>`
4. `$XDG_DATA_HOME/shiromino`
5. `$HOME/.local/share/shiromino`
6. `/usr/local/share/shiromino`
7. `/usr/share/shiromino`

Any path in this list will match only if it contains a directory named `data`.

The first path that matches will be honored.

You can also specify a custom `SHARE_PATH` in the configuration file. If `SHARE_PATH` is relative, the path is assumed to be relative to the configuration file.
### `CACHE_PATH`
`CACHE_PATH` is a path to a directory which contains dynamic game data such as high scores.

If `CACHE_PATH` is left unspecified, the game will look for it here (in order):

1. `<executable directory>`
2. `<executable directory>/../var/cache/shiromino`
3. `<current working directory>`
4. `$XDG_CACHE_HOME/shiromino`
5. `$HOME/.cache/shiromino`
6. `/usr/local/var/cache/shiromino`
7. `/var/cache/shiromino`

Any path in this list will match only if it contains a file named `shiromino.sqlite`.

The first path that matches will be honored.

If none match, the game will try to create the following directories and use them (in order):

1. `$XDG_CACHE_HOME/shiromino`
2. `$HOME/.cache/shiromino`
3. `<executable directory>`

You can also specify a custom `CACHE_PATH` in the configuration file. If `CACHE_PATH` is relative, the path is assumed to be relative to the configuration file.