# Simple script to embed Lua files as C header files
import os

LUPCK_LIB_FILE = "lua/lupck/init.lua"
LUPCK_HEADER_FILE = "include/lupck/lupck_embedded_lua.h"

def embed_file(lua_path, header_path):
    with open(lua_path, 'rb') as f:
        content = f.read()

    name = os.path.basename(lua_path).replace('.', '_')

    with open(header_path, 'w') as f:
        f.write("#pragma once\n\n")
        f.write(f"const unsigned char LUPCK_INIT_LUA[] = {{\n    ")
        # Converts each byte to a two-digit hex string
        hex_data = [f"0x{b:02x}" for b in content]
        f.write(", ".join(hex_data))
        f.write("\n};\n\n")
        f.write(f"const unsigned int LUPCK_INIT_LUA_LEN = {len(content)};\n")
        print(f"Embedded {name} ({len(content)} bytes)")

if __name__ == "__main__":
    embed_file(LUPCK_LIB_FILE, LUPCK_HEADER_FILE)
