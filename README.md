# mindwmstat

[![builds.sr.ht status](https://builds.sr.ht/~kovmir/mindwmstat/commits/master/.build.yml.svg)](https://builds.sr.ht/~kovmir/mindwmstat/commits/master/.build.yml?)

Minimal [dwm](https://dwm.suckless.org/) status bar

# PREVIEW

```
 +---- RAM Usage                             Power
 |                                             |
 v                                             v
5% | 0.00 0.01 0.02 | 2026-04-15 Wed 09:31 | +[92%]
           ^                   ^
           |                   |
       CPU Load            Date & Time
```

![screenshot](screenshot.png)

*Power indicator disappears on non–battery-powered devices.*

# INSTALL

```sh
git clone https://git.sr.ht/~kovmir/mindwmstat
cd mindwmstat
make # Optionally adjust config.h to your liking.
sudo make install

# make CONSOLE=1   # Print status bar to the console.
# make ANIMATION=1 # Play animation in status bar, see config.h.
# make ONCE=1      # Set status only once and terminate.
```

# USAGE

```sh
mindwmstat # Put 'mindwmbar &' in your '~/.xinitrc'.
```

# DEPENDENCIES

* [GNU Make](https://www.gnu.org/software/make/)
* [pkg-config](https://gitlab.freedesktop.org/pkg-config/pkg-config)
* [GCC](https://gcc.gnu.org/) or [Clang](https://clang.llvm.org/)
* [Xlib](https://www.x.org/releases/current/doc/libX11/libX11/libX11.html)

# SUPPORTED OS

* Linux

# CONTRIBUTING

When submitting PRs, please maintain the [coding
style](https://suckless.org/coding_style/) used for the project.
