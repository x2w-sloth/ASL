# Another System Language

ASL is yet another system language designed for low-level programming. Despite being a "toy" language, it aims to provide a minimal but complete set of language features to be used to develop any serious system software, such as kernels and device drivers.

## ASLC

ASLC, the ASL compiler, is a naive two-pass compiler written in C to compile ASL programs while the language specification is still in progress. ASLC uses the GNU assembler and linker as its backend and does not rely on other libraries or frameworks such as LLVM, Lex, or Yacc. ASLC currently emits verbose (yet correct) assembly as there are no optimization passes.

ASLC is built with a GNU Makefile by invoking `make aslc`. You can run tests with `make test` and find some small example ASL programs in the `test/asl` directory.

## Toolchain

Some tools that facilitate ASL programming can be found in the `util` directory. There is syntax highlighting support for vim and neovim.

## License

This repository is distributed under the MIT license.
