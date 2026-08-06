// ProjectTXT.cpp – tree + file list with contents
// Compile: g++ -std=c++17 ProjectTXT.cpp -o ProjectTXT   (Linux/macOS)
//          cl /EHsc /std:c++17 ProjectTXT.cpp            (Windows)

#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <limits.h>
#endif

namespace fs = std::filesystem;

// ----------------------------------------------------------------------
// Default exclusion list – compiled into the binary
// Change these to your own default patterns.
const std::vector<std::string> DEFAULT_EXCLUDES = {
    ".cache",
    "obj",
    "files/map"
};

// ----------------------------------------------------------------------
// Trim leading/trailing whitespace
static std::string trim(const std::string &s) {
    size_t start = 0;
    while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) ++start;
    size_t end = s.size();
    while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1]))) --end;
    return s.substr(start, end - start);
}

// ----------------------------------------------------------------------
// Return the directory containing the running executable
static fs::path get_executable_dir() {
#ifdef _WIN32
    wchar_t path[MAX_PATH] = {0};
    GetModuleFileNameW(nullptr, path, MAX_PATH);
    return fs::path(path).parent_path();
#else
    char result[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
    if (count != -1) {
        return fs::path(std::string(result, count)).parent_path();
    }
    // Fallback if /proc is not available (e.g. macOS – you can extend this)
    return fs::current_path();
#endif
}

// ----------------------------------------------------------------------
// Check whether 'path' (relative to 'root') matches any exclude pattern
static bool is_excluded(const fs::path &path, const fs::path &root,
                        const std::vector<std::string> &patterns) {
    if (patterns.empty()) return false;
    std::error_code ec;
    fs::path rel = fs::relative(path, root, ec);
    if (ec) return false;                     // can't determine → don't exclude
    std::string rel_str = rel.generic_string();   // forward slashes
    if (rel_str.empty()) return false;        // the root itself

    for (const auto &pat : patterns) {
        if (rel_str == pat) return true;
        if (rel_str.compare(0, pat.size(), pat) == 0 &&
            rel_str.size() > pat.size() && rel_str[pat.size()] == '/')
            return true;
    }
    return false;
}

// ----------------------------------------------------------------------
// Simple recursive tree printer (like the 'tree' command)
void print_tree(std::ostream &out, const fs::path &dir,
                const std::string &prefix, bool is_last,
                const fs::path &start_dir,
                const std::vector<std::string> &exclude_patterns) {
    // Print the current node
    out << prefix << (is_last ? "└── " : "├── ") << dir.filename().string() << '\n';

    if (!fs::is_directory(dir)) return;

    std::vector<fs::directory_entry> entries;
    try {
        for (const auto &e : fs::directory_iterator(dir)) {
            // Skip excluded directories completely
            if (e.is_directory() && is_excluded(e.path(), start_dir, exclude_patterns))
                continue;
            entries.push_back(e);
        }
    } catch (...) { return; }

    // Sort: directories first, then files, alphabetically
    std::sort(entries.begin(), entries.end(),
              [](const fs::directory_entry &a, const fs::directory_entry &b) {
                  if (a.is_directory() != b.is_directory())
                      return a.is_directory();  // directories first
                  return a.path().filename() < b.path().filename();
              });

    for (size_t i = 0; i < entries.size(); ++i) {
        bool last_child = (i == entries.size() - 1);
        std::string child_prefix = prefix + (is_last ? "    " : "│   ");
        print_tree(out, entries[i].path(), child_prefix, last_child,
                   start_dir, exclude_patterns);
    }
}

// ----------------------------------------------------------------------
// Quick check if a file is likely binary (contains a null byte)
static bool is_binary(const fs::path &filepath) {
    std::ifstream f(filepath, std::ios::binary);
    if (!f) return false;
    char buf[512];
    f.read(buf, sizeof(buf));
    auto count = f.gcount();
    for (auto i = 0; i < count; ++i) {
        if (buf[i] == '\0') return true;
    }
    return false;
}

// ----------------------------------------------------------------------
int main() {
    // Directories to scan for file contents
    const std::vector<std::string> dirs = {"src", "obj", "shaders", "include"};

    // --- 1. Determine which exclusion list to use ------------------------------
    std::vector<std::string> exclude_patterns;
    fs::path exe_dir = get_executable_dir();
    fs::path external_exclude = exe_dir / "exclude.txt";

    if (fs::exists(external_exclude)) {
        std::ifstream f(external_exclude);
        if (f) {
            std::string line;
            while (std::getline(f, line)) {
                std::string pat = trim(line);
                if (pat.empty()) continue;
                // Remove trailing slash or backslash
                while (!pat.empty() && (pat.back() == '/' || pat.back() == '\\'))
                    pat.pop_back();
                if (!pat.empty())
                    exclude_patterns.push_back(pat);
            }
            std::cout << "Loaded external exclude.txt from: " << external_exclude << '\n';
        }
    }
    if (exclude_patterns.empty()) {
        exclude_patterns = DEFAULT_EXCLUDES;
        std::cout << "Using compiled-in default excludes.\n";
    }

    // --- 2. Ensure output directory exists ------------------------------------
    std::error_code ec;
    if (!fs::create_directory("output", ec) && ec) {
        if (ec != std::make_error_code(std::errc::file_exists)) {
            std::cerr << "Error creating output directory: " << ec.message() << '\n';
            return 1;
        }
    }

    // Open output file
    std::ofstream out("ProjectTXT.txt");
    if (!out) {
        std::cerr << "Error: Could not open ProjectTXT.txt for writing.\n";
        return 1;
    }

    // --- 3. Full directory tree -----------------------------------------------
    out << "=== DIRECTORY TREE ===\n";
    fs::path root = fs::current_path();
    print_tree(out, root, "", true, root, exclude_patterns);
    out << "\n\n";

    // --- 4. File list with contents -------------------------------------------
    out << "=== FILES AND CONTENTS ===\n\n";

    for (const auto &dir : dirs) {
        fs::path dirPath(dir);

        if (!fs::exists(dirPath) || !fs::is_directory(dirPath)) {
            out << "[Warning: '" << dir << "' does not exist or is not a directory – skipping]\n\n";
            std::cerr << "Warning: '" << dir << "' missing – skipping.\n";
            continue;
        }

        try {
            for (const auto &entry : fs::recursive_directory_iterator(dirPath)) {
                if (!entry.is_regular_file()) continue;

                // Skip files that belong to excluded directories
                if (is_excluded(entry.path(), root, exclude_patterns))
                    continue;

                std::string fpath = entry.path().string();
                out << "--- File: " << fpath << " ---\n";

                if (is_binary(entry.path())) {
                    out << "[Binary file, content not displayed]\n";
                } else {
                    std::ifstream fin(entry.path(), std::ios::binary);
                    if (!fin) {
                        out << "[Error opening file]\n";
                    } else {
                        out << fin.rdbuf();
                        if (fin.bad())
                            out << "\n[Error reading file]";
                    }
                }
                out << "\n--- End of file: " << fpath << " ---\n\n";
            }
        } catch (const fs::filesystem_error &e) {
            out << "[Error while reading '" << dir << "': " << e.what() << "]\n\n";
            std::cerr << "Error in '" << dir << "': " << e.what() << '\n';
        }
    }

    out.close();
    std::cout << "Tree and file contents written to ProjectTXT.txt\n";
    return 0;
}