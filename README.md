# FlorrDefence

A game combines Tower Defence and florr.io. 

## Build (Cmake)

### Clone

```bash
git clone --recurse-submodules https://github.com/YunTianZhou/FlorrDefence.git
cd FlorrDefence
mkdir build
cd build
```

### Config

```bash
cmake ..
```

Specify generator (e.g. Visual Studio 2022):

```bash
cmake -G "Visual Studio 17 2022" -A x64 ..
```

### Generate

```bash
cmake build --config Release
```
