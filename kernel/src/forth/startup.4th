
: latest dictionary @ ;
: nt>flags 8 + ;
: immediate true latest nt>flags c! ;

: postpone ' , ; immediate

: char 20 parse drop c@ ;
: [char] char postpone literal ; immediate
: ( [char] ) parse 2drop ; immediate
: \ a parse 2drop ; immediate

\ Now we have comments!
: ['] ' postpone literal ; immediate

: . ( n -- ) forth_c_print ccall1 drop ;
: emit ( c -- ) putchar ccall1 drop ;
: type ( caddr u -- ) write ccall2 drop ;
: .( [char] ) parse type ; immediate

: cr a emit ;

: nop ( comment?? ) ;

\ Control flow
: prepare> ( -- fwd ) here 0 , ;
: resolve> ( fwd -- ) here swap ! ;
: <prepare ( -- rev ) here ;
: <resolve ( rev -- ) , ;

: if ['] 0branch , prepare> ; immediate
: else ['] branch , prepare> swap resolve> ; immediate
: then resolve> ; immediate

: begin <prepare ; immediate
: again ['] branch , <resolve ; immediate
: while postpone if swap ; immediate
: repeat postpone again postpone then ; immediate
