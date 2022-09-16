# Pomodoro Timer for TimeWarrior

***
Pomodoro timer is a timer interface executing `timew continue` and `timew stop`
commands to mimic pomodoro sessions with
[timewarrior](https://github.com/GothenburgBitFactory/timewarrior) as the backend.

This is intended to be used with
[taskwarrior](https://github.com/GothenburgBitFactory/taskwarrior) and the hook
script that integrates it with
[timewarrior](https://github.com/GothenburgBitFactory/timewarrior). You can learn
more information in their docs.

## Requirements
- ncurses
- OpenAL (For desktop)
- OpenSLES (For embedded devices)

## Installation
```bash
git clone https://github.com/OmarMohamedKhallaf/Timewarrior-Pomodoro.git pomo
cd pomo
# Use -DCMAKE_INSTALL_PREFIX=$PREFIX/usr for termux on android
cmake -B build -S ./ -DCMAKE_INSTALL_PREFIX=/usr/local -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel 4 --target install
```

## Task lists
- [x] Add sounds after at the end of work and break sessions
- [x] Parse output from child process
- [x] Adapt to changes in terminal size
- [x] Support unicode
- [ ] Handle errors properly (is timew installed ?)
- [ ] Make variables configurable
- [ ] Confirm exit before exiting
- [ ] Support for `timew start <tags...>` in the interface
- [ ] Automatic session tracking by handling signals (e.g. from taskwarrior hook scripts)
- [ ] Use ascii art to print digits adapted to the size of the terminal

## Usage
There are only three commands for now
```text
s: to start a session followed by a break
p: to pause the current session (actually stops it in timewarrior terms)
e: to exit
```

## Contributing
Pull requests are welcome. For major changes, please open an issue first to discuss what you would like to change.

## License
[GNU General Public License v3.0](https://choosealicense.com/licenses/gpl-3.0/)
