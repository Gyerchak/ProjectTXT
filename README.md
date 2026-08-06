# ProjectTXT – Directory Tree & File Listing Tool (C++20)

ProjectTXT scans a project folder, prints a full directory tree (respecting
excluded paths), and dumps the contents of text files found in `src`, `obj`,
`shaders`, and `include` into a single `ProjectTXT.txt` report.

---

## Features

- Recursive directory tree (mimics the `tree` command).
- File content dump for selected directories; binary files are skipped.
- **Smart exclusion system**:
  - **Built‑in defaults** – compiled into the binary.
  - **Runtime override** – place an `exclude.txt` next to the executable; it
    replaces the defaults automatically.
- Written in **C++20**, using `std::filesystem` and `std::string::ends_with`.

---

## Build on Linux

You need a compiler with C++20 support (GCC ≥ 10, Clang ≥ 10).

```bash
g++ -std=c++20 ProjectTXT.cpp -o ProjectTXT