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
- sdl2
- sdl2_mixer

## Installation
```bash
git clone https://github.com/OmarMohamedKhallaf/Timewarrior-Pomodoro.git
cd Timewarrior-Pomodoro
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
sudo make install/local
```

## Task lists
- [x] Add sounds after at the end of work and break sessions
- [x] Handle errors properly
- [ ] Parse output from child process
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
