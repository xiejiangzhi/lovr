#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_STDIO
#define STBI_ONLY_JPEG
#define STBI_ONLY_PNG
#define STBI_ONLY_HDR
#define STBI_ASSERT(x)
#define STBI_MALLOC lovrMalloc
#define STBI_REALLOC lovrRealloc
#define STBI_FREE lovrFree
#include "util.h"
#include "stb_image.h"
