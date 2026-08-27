#include <cstdio>
#include <cstring>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: WarehouseValidator <file_path>\n");
        return 1;
    }

    const char* filePath = argv[1];

    //otvaramo datoteku
    FILE* f = fopen(filePath, "rb");
    if (!f) {
        fprintf(stderr, "Error: Cannot open file: %s\n", filePath);
        return 1;
    }

    //citamo prvih 16 bajtova zaglavlja
    char buffer[16];
    size_t bytesRead = fread(buffer, 1, sizeof(buffer), f);
    fclose(f);

    if (bytesRead < sizeof(buffer)) {
        fprintf(stderr, "Error: File too small to be a valid SQLite database\n");
        return 1;
    }

    //SQLite magic: "SQLite format 3" + jedan null bajt = 16 bajtova
    if (memcmp(buffer, "SQLite format 3\0", 16) != 0) {
        fprintf(stderr, "Error: Not a valid SQLite database (bad header)\n");
        return 1;
    }

    printf("OK: Valid SQLite database\n");
    return 0;
}
