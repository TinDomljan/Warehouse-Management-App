#include <cstdio>
#include <cstring>
#include <ctime>


struct SnapshotHeader {
    char   magic[4];
    int    version;
    int    recordCount;
    time_t timestamp;
};

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

    SnapshotHeader header;

    size_t bytesRead = fread(&header, sizeof(SnapshotHeader), 1, f);
    fclose(f);


    //cetiri provjere
    if (bytesRead != 1) {
        fprintf(stderr, "Error: File too small to be a valid snapshot\n");
        return 1;
    }

    if (memcmp(header.magic, "WHSE", 4) != 0) {
        fprintf(stderr, "Error: Invalid file format (bad magic bytes)\n");
        return 1;
    }

    if (header.version != 1) {
        fprintf(stderr, "Error: Unsupported snapshot version: %d\n", header.version);
        return 1;
    }

    if (header.recordCount < 0) {
        fprintf(stderr, "Error: Corrupted record count\n");
        return 1;
    }

    printf("OK: Valid snapshot — %d product(s), version %d\n",
           header.recordCount, header.version);
    return 0;
}
