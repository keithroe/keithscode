
set nocompatible
syntax on
filetype on
filetype plugin on
filetype indent on
 
set history=1000
set showmatch
set matchtime=0
set shortmess=atI
set ruler
set showcmd
 
set sidescroll=1
set sidescrolloff=3
 
" Set xterm title
set title
 
" Search options
set hlsearch
set incsearch
"set ignorecase
set smartcase
 
set visualbell
set noerrorbells
set printoptions=paper:letter
 
" Keep more lines of context
set scrolloff=3
 
" Make backspace delete lots of things
set backspace=indent,eol,start
 
" Auto-backup files and .swp files don't go to pwd
set backupdir=~/.vim-tmp,~/.tmp,~/tmp,/var/tmp,/tmp
set directory=~/.vim-tmp,~/.tmp,~/tmp,/var/tmp,/tmp
 
" Allow switching edited buffers without saving
set hidden
 
" Look for the file in the current directory, then south until you reach home.
set tags=tags;~/
 
" Change <Leader>
let mapleader = ","
 
" Alternatives to using Ctrl-[ for esc:
"imap jkl <esc>
"imap jlk <esc>
"imap kjl <esc>
"imap klj <esc>
"imap lkj <esc>
"imap ljk <esc>
"imap ;l <esc>
 
" Quick timeouts on key combinations.
set timeoutlen=300
 
":nmap : q:i
":nmap / q/i
":nmap ? q?i
 
" Who needs .gvimrc?
if has('gui_running')
  set encoding=utf-8
  "set guifont=Monospace\ Bold\ 9
  set guifont=Bitstream\ Vera\ Sans\ Mono\ 8
  " Turn off toolbar and menu
  set guioptions-=T
  set guioptions-=m
  colorscheme ir_black 
else
  colorscheme ir_black 
end
 
" Lusty
"let g:LustyJugglerShowKeys = 2
let g:LustyExplorerSuppressRubyWarning = 1
let g:LustyExplorerFileMasks = "*.o,*.fasl"
nmap <silent> <Leader>f :FilesystemExplorer<CR>
nmap <silent> <Leader>b :BufferExplorer<CR>
nmap <silent> <Leader>r :FilesystemExplorerFromHere<CR>
nmap <silent> <Leader>g :LustyJuggler<CR>
nmap <silent> <Leader>j :so ~/.vim/plugin/lusty-explorer.vim<CR>
nmap <silent> <TAB> :JugglePrevious<CR>
 
" Window management
"nmap <silent> <Leader>xo :wincmd j<CR>
 
" For lisp repl
"autocmd BufRead *.lsp,*.lisp so ~/vilisp/VIlisp.vim
 
" Catch trailing whitespace
set listchars=tab:>-,trail:·,eol:$
nmap <silent> <leader>s :set nolist!<CR>
 
" Fix command typos (stolen from Adam Katz)
nmap ; :
 
" ` is more useful than ' but less accessible.
nnoremap ' `
nnoremap ` '
 
" Buffer management
nmap <C-h> :bp<CR>
nmap <C-l> :bn<CR>
"nmap <TAB> :b#<CR>
"nmap <C-q> :bd<CR>
"nmap <C-d> :bw<CR>
 
" Toggle highlighting
" nmap <silent> <C-n> :silent :set nohls!<CR>:silent :set nohls?<CR>
nmap <silent> <C-n> :silent :nohlsearch<CR>

" Press Space to turn off highlighting and clear any message already
" displayed.
noremap <silent> <Space> :silent noh<Bar>echo<CR>

nmap H ^
nmap L $
 
" Scroll faster
nnoremap <C-e> 3<C-e>
nnoremap <C-y> 3<C-y>
vnoremap <C-e> 3<C-e>
vnoremap <C-y> 3<C-y>
 
" % matches on if/else, html tags, etc.
runtime macros/matchit.vim
 
" Bash-like filename completion
set wildmenu
set wildmode=list:longest
 
autocmd BufRead *.qcf set filetype=lisp
autocmd BufRead qpx.inc set filetype=make
autocmd BufRead letter* set filetype=mail
autocmd Filetype mail set fo -=l autoindent spell

set tabstop=2 softtabstop=2 shiftwidth=2 expandtab autoindent
set expandtab
" ITA indenting style
autocmd Filetype c,cpp,cc,h set tabstop=2 softtabstop=2 shiftwidth=2 expandtab
" JSH indenting style
"autocmd Filetype c,cpp,h set tabstop=8 softtabstop=4 shiftwidth=4 noexpandtab
" Wimba indenting style, sort-of
"autocmd Filetype c,cpp,h set tabstop=2 softtabstop=2 shiftwidth=2 expandtab
" Viewglob
"autocmd Filetype c,cpp,h,sh set cindent autoindent
 
"autocmd Filetype sh set ts=4 shiftwidth=2 expandtab
"autocmd Filetype lisp,ruby,xml,html set ts=8 shiftwidth=2 expandtab
"autocmd Filetype xml,xslt,diff,ruby color desert
"autocmd BufReadPre .viper set filetype=lisp
 
" a.vim
nmap <silent> <Leader>h :A<CR>
 
if has("autocmd")
  autocmd BufReadPost *
        \ if line("'\"") > 0 && line("'\"") <= line("$") |
        \ exe "normal g'\"" |
        \ endif
endif

" Indent XML readably
function! DoPrettyXML()
  1,$!xmllint --format --recover -
endfunction
command! PrettyXML call DoPrettyXML()

au BufNewFile,BufRead *.cu set ft=cuda


