# Chimera Spoofer

Chimera Spoofer is a powerful utility for Linux designed to enhance privacy and anonymity by modifying system-level identifiers. It can spoof and restore various fingerprints, including hostname, machine ID, OS release, SSH keys, network settings, and file timestamps.

## ⚠️ Disclaimer

This software is provided "as is" without any warranties. By using this software, you agree to the following terms:

- You are solely responsible for any potential damage or system incompatibilities that may arise from its use.
- The owner and developer, LimmeDev, is not responsible for and is not concerned with how you use this software. Any actions you take are your own responsibility.

## 🛠️ Building from Source

### Prerequisites

* Python ≥ 3.8 (needed only for the build tool)
* `clang++` or `g++` with C++17 support

Mint, the minimal build system, is now our single source-of-truth for compiling Chimera Spoofer. Install it once via `pip`:

```bash
python -m pip install --upgrade mint-build   # requires Mint ≥ 0.2.0
```

### Build

```bash
# Clone the repository
git clone https://github.com/LimmeDev/ChimeraSpoofer.git
cd ChimeraSpoofer

# Release build (-O3) – binary will be placed in ./build/…/chimera
mint build -r
```

The legacy `compile.sh` now acts as a thin wrapper around the same command, so

```bash
./compile.sh   # does exactly the same as `mint build -r`
```

This will generate the `chimera` executable and the `interceptor.so` shared library.

## 🚀 Usage

There are two primary ways to use Chimera Spoofer:

1.  **Direct Execution**: Run the main executable with root privileges.
    ```bash
    sudo ./chimera
    ```

2.  **Interceptor**: Use the `chimera.sh` script to launch an application with the spoofer's interceptor preloaded.
    ```bash
    ./chimera.sh <your_command_here>
    ```
    For example, to launch `google-chrome` with the interceptor:
    ```bash
    ./chimera-browser.sh
    ```

## 📜 License

This project is under a private, proprietary license. You are permitted to use the software, but you are **not allowed to copy, edit, distribute, or reverse-engineer it.**

For full license details, please see the [LICENSE](LICENSE) file.

---
*Developed by LimmeDev* 