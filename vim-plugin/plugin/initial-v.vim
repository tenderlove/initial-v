let s:path = fnamemodify(resolve(expand('<sfile>:p')), ':h')
let s:cli = fnamemodify(s:path .. "/../../client/zig-out/bin/initial-v", ":p")

fun! Lost()
  echom "Lost"
  call job_start([s:cli, "general"])
endfun

fun! Gained()
  echom "Gained"
  call job_start([s:cli, "vim"])
endfun

if filereadable(s:cli)
  augroup InitialV
    autocmd!
    "autocmd FocusGained * :call job_start([s:cli, "vim"])
    autocmd FocusGained * :call Gained()
    autocmd GUIEnter * :call Gained()
    "autocmd FocusLost * :call job_start([s:cli, "general"])
    autocmd FocusLost * :call Lost()
    autocmd InsertEnter * :silent call job_start([s:cli, "neutral"])
    autocmd InsertLeave * :silent call job_start([s:cli, "drive"])
    autocmd BufWritePost * :silent call job_start([s:cli, "park"])
  augroup END
else
  echom "wat"
endif
