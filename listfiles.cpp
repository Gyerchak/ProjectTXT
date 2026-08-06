// listfiles.cpp – tree + file list with contents
#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <vector>
#include <algorithm>   // ← needed for std::sort

namespace fs = std::filesystem;

// ----------------------------------------------------------------------
// Simple recursive tree printer (like the 'tree' command)
void print_tree(std::ostream &out, const fs::path &dir,
                const std::string &prefix = "", bool is_last = true) {
    out << prefix << (is_last ? "└── " : "├── ") << dir.filename().string() << '\n';

    if (!fs::is_directory(dir)) return;

    std::vector<fs::directory_entry> entries;
    try {
        for (const auto &e : fs::directory_iterator(dir))
            entries.push_back(e);
    } catch (...) { return; }

    // Sort entries: directories first, then files, alphabetically
    std::sort(entries.begin(), entries.end(),
              [](const fs::directory_entry &a, const fs::directory_entry &b) {
                  if (a.is_directory() != b.is_directory()) {
                      return a.is_directory();  // directories first
                  }
                  return a.path().filename() < b.path().filename();
              });

    for (size_t i = 0; i < entries.size(); ++i) {
        bool last_child = (i == entries.size() - 1);
        std::string child_prefix = prefix + (is_last ? "    " : "│   ");
        print_tree(out, entries[i].path(), child_prefix, last_child);
    }
                }

                // ----------------------------------------------------------------------
                // Quick check if a file is likely binary (contains a null byte)
                bool is_binary(const fs::path &filepath) {
                    std::ifstream f(filepath, std::ios::binary);
                    if (!f) return false;               // can't open → treat as text
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
                    // Directories to scan (relative to current working directory)
                    const std::vector<std::string> dirs = {"src", "obj", "shaders", "include"};

                    // Ensure output directory exists
                    std::error_code ec;
                    if (!fs::create_directory("output", ec) && ec) {
                        // If it fails for a reason other than "already exists", report error
                        if (ec != std::make_error_code(std::errc::file_exists)) {
                            std::cerr << "Error creating output directory: " << ec.message() << '\n';
                            return 1;
                        }
                    }

                    // Open output file
                    std::ofstream out("output/filelist.txt");
                    if (!out) {
                        std::cerr << "Error: Could not open output/filelist.txt for writing.\n";
                        return 1;
                    }

                    // --- 1. Full directory tree of current folder ---
                    out << "=== DIRECTORY TREE ===\n";
                    print_tree(out, fs::current_path(), "", true);
                    out << "\n\n";

                    // --- 2. File list with contents ---
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

                                std::string fpath = entry.path().string();
                                out << "--- File: " << fpath << " ---\n";

                                if (is_binary(entry.path())) {
                                    out << "[Binary file, content not displayed]\n";
                                } else {
                                    // Stream the file content (handles large files gracefully)
                                    std::ifstream fin(entry.path(), std::ios::binary);
                                    if (!fin) {
                                        out << "[Error opening file]\n";
                                    } else {
                                        out << fin.rdbuf();   // write entire content
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
                    std::cout << "Tree and file contents written to output/filelist.txt\n";
                    return 0;
                }
