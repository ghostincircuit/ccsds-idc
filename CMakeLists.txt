cmake_minimum_required (VERSION 2.6)
project (ccsds-idc)

add_executable(bitset src/bitset.c)
target_compile_definitions(bitset PUBLIC _TEST_BITSET_)

add_executable(bpe_block src/bpe_block.c src/image_loader.c)
target_compile_definitions(bpe_block PUBLIC _TEST_BPE_BLOCK_)

add_executable(bpe_segment src/bpe_segment.c src/bpe_block.c src/bitset.c)
target_compile_definitions(bpe_segment PUBLIC _TEST_BPE_SEGMENT_)