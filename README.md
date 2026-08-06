# ProjectTXT – Directory Tree & File Listing Tool (C++20)

ProjectTXT scans a project folder, prints a full directory tree, and dumps
the contents of text files found in `src`, `obj`, `shaders`, and `include`
into a single `ProjectTXT.txt` report.

---

## Features

- **Recursive directory tree** – directories sorted before files, alphabetically.
- **File content dump** – skips binary files automatically.
- **Smart exclusion system**:
  - **Built‑in defaults** – `.git` and `.cache` are always excluded (hardcoded
    in the binary).
  - **Runtime additions** – place an `exclude.txt` in your **project root**
    (same folder where you run `./ProjectTXT`). The patterns listed there are
    added to the built‑in list – they do not replace the defaults.
- **Consistent exclusion** – excluded directories are removed from the tree,
  and any files inside them are not listed in the content dump.

---

## Build on Linux

Requires a C++20 compiler (GCC ≥ 10, Clang ≥ 10).

```bash
g++ -std=c++20 ProjectTXT.cpp -o ProjectTXT