# Pomodoro Timer for Timewarrior
***
Pomodoro timer is a timer interface executing `timew continue` and `timew stop`
commands to mimic pomodoro sessions with
[timewarrior](https://github.com/GothenburgBitFactory/timewarrior) as the backend.

This is intended to be used with
[taskwarrior](https://github.com/GothenburgBitFactory/taskwarrior) and the hook
script that integrates it with
[timewarrior](https://github.com/GothenburgBitFactory/timewarrior). You can more
information in their docs.

## Installation
```bash
git clone https://github.com/OmarMohamedKhallaf/Timewarrior-Pomodoro.git
cd Timewarrior-Pomodoro
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
sudo make install/local
```

## Task lists
- [ ] Handle errors properly
- [ ] Make variables configurable
- [ ] Confirm exit before exiting

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