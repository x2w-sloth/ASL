# Utilities

Some utilities to make ASL more accessible.

## Syntax Highlighting

Syntax highlighting is available for vim and neovim.

### Vim Users

From the current directory `util/vim`, please run the following commands to install the syntax file.

```bash
mkdir -p ~/.vim/syntax && cp vim/syntax/asl.vim ~/.vim/syntax/
mkdir -p ~/.vim/ftdetect && cp vim/ftdetect/asl.vim ~/.vim/ftdetect/
```

### Neovim Users

Instead of copying to the `~/.vim` directory, please use the `~/.config/nvim` directory, or your custom Neovim root directory.

```bash
mkdir -p ~/.config/nvim/syntax && cp vim/syntax/asl.vim ~/.config/nvim/syntax/
mkdir -p ~/.config/nvim/ftdetect && cp vim/ftdetect/asl.vim ~/.config/nvim/ftdetect/
```
