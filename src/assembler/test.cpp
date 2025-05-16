#include <cstdlib>
#include <string>

#include "../../include/assembler/assembler.hpp"

int TestAssembler() {

  std::string source = R"(; Test program for tokenizer coverage

Start:
    CLS
    RET
    JP Start
    JP V0, Start
    JP Start
    JP V0, 0x222
    JP 0x333
    CALL Start
    CALL 0x200
    SE V0, 0x22
    SE V0, V1
    SNE V0, 0x22
    SNE V0, V1
    ADD V0, 0x22
    ADD V0, V1
    ADD I, VF
    AND V1, V2
    OR V2, V3
    XOR V3, V4
    SUB V3, V4
    SUBN V3, V4
    LD V3, 0x22
    LD V0, V1
    LD I, 0x222
    LD I, Start
    LD I, name
    LD [I], V0
    LD V1, [I]
    LD V5, DT
    LD ST, V4
    LD DT, V2
    LD F, V0
    LD B, V1
    LD V4, K
    RND V0, 0x22
    SKP V0
    SKNP V9
    DRW V0, V1, 5
    SHR V2
    SHL V1
name:
    RET
)";
  Assembler chip8_asm(source);

  return EXIT_SUCCESS;
}
