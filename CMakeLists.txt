cmake_minimum_required (VERSION 2.6)
project (ccsds-idc)

SET(CMAKE_C_FLAGS "-Wall -O0 -g")

add_executable(bitset src/bitset.c)
target_compile_definitions(bitset PUBLIC _TEST_BITSET_)

add_executable(bpe_block src/bpe_block.c src/image_loader.c)
target_compile_definitions(bpe_block PUBLIC _TEST_BPE_BLOCK_)

add_executable(bpe_encode src/bpe_encode.c src/bpe_block.c src/bitset.c)
target_compile_definitions(bpe_encode PUBLIC _TEST_BPE_ENCODE_)