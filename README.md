# ProjectTXT – Directory Tree & File Listing Tool

**ProjectTXT** scans a project folder, prints a full directory tree (respecting
excluded paths), and dumps the contents of text files found in `src`, `obj`,
`shaders`, and `include` into a single `ProjectTXT.txt` report.  
It’s useful for quickly sharing a project’s structure and source code in one
concise text file – e.g., for LLM prompts, documentation, or code reviews.

---

## Features

- **Recursive directory tree** – mimics the `tree` command, with directories
  sorted before files and alphabetically.
- **File content dump** – reads every file in the predefined directories and
  writes its contents to the output. Binary files are detected and skipped.
- **Smart exclusion system**:
  - **Default built‑in list** – compiled into the executable. Works without any
    external configuration file.
  - **Runtime override** – place an `exclude.txt` **in the same folder as the
    executable** to replace the defaults. The tool picks it up automatically,
    no recompilation needed.
- **Cross‑platform** – works on Linux, macOS, and Windows (the code includes
  the necessary OS‑specific path retrieval).

---

## File Structure
project/
├── ProjectTXT.cpp # Main source code
├── exclude.txt # (optional) Override default exclusion patterns
├── ProjectTXT # Compiled executable
├── ProjectTXT.txt # Generated output (created at runtime)
├── output/ # Created by the tool (currently unused, placeholder)
├── src/ # Example source directory (scanned)
├── obj/ # Excluded by default
├── include/ # Example include directory
└── shaders/ # Example shader directory

text

---

## Build on Linux

Make sure you have a C++17 capable compiler (GCC ≥ 8 or Clang ≥ 5) and the
standard library with `<filesystem>` support.  

If using `g++`, link the `stdc++fs` library if required (on older systems;
modern distributions include it automatically).

```bash
g++ -std=c++17 ProjectTXT.cpp -o ProjectTXT