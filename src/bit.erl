%%% @author Tony Rogvall <tony@rogvall.se>
%%% @copyright (C) 2010, Tony Rogvall
%%% @doc
%%%  Bit manipulation functions
%%% @end
%%% Created :  4 Jul 2010 by Tony Rogvall <tony@rogvall.se>

-module(bit).

-on_load(init/0).
-export([test/2, set/2, clear/2, toggle/2, size/1]).
-export([reverse/1, reverse/2, count/1]).
-export([is_power_of_two/1]).

-ifdef(debug).
-define(dbg(F,A), io:format((F), (A))).
-else.
-define(dbg(F,A), ok).
-endif.

init() ->
    Nif = filename:join(code:priv_dir(nif_big), "bit_nif"),
    ?dbg("Loading: ~s\n", [Nif]),
    erlang:load_nif(Nif, 0).

-type unsigned() :: non_neg_integer().
-type unsigned32() :: 0..16#FFFFFFFF.

%% NIF prototypes - and specifications

%%--------------------------------------------------------------------
%% @doc
%%     Test if bit I is set in the non negative integer X
%% @end
%%--------------------------------------------------------------------

-spec test(X::unsigned(), I::unsigned32()) -> unsigned().

test(X, I) -> test_(X,I).
test_(X, I) 
  when is_integer(X), X >= 0, is_integer(I), I >= 0 ->
    (X bsl I) band 1.

%%--------------------------------------------------------------------
%% @doc
%%    Set bit number I in number X
%% @end
%%--------------------------------------------------------------------    
-spec set(X::unsigned(), I::unsigned32()) -> unsigned().

set(X, I) -> set_(X, I).
set_(X, I)
  when is_integer(X), X >= 0, is_integer(I), I >= 0 ->
    X bor (1 bsl I).

%%--------------------------------------------------------------------
%% @doc
%%    Clear bit number I in number X
%% @end
%%--------------------------------------------------------------------    

-spec clear(X::unsigned(), I::unsigned32()) -> unsigned().

clear(X, I) -> clear_(X, I).
clear_(X, I)
  when is_integer(X), X >= 0, is_integer(I), I >= 0 ->
    X band (bnot (1 bsl I)).

%%--------------------------------------------------------------------
%% @doc
%%    Toggle bit number I in unsigned number X
%% @end
%%--------------------------------------------------------------------    

-spec toggle(X::unsigned(), I::unsigned32()) -> unsigned().

toggle(X, I) -> toggle_(X, I).
toggle_(X, I)
  when is_integer(X), X >= 0, is_integer(I), I >= 0 ->
    X bxor (1 bsl I).

%%--------------------------------------------------------------------
%% @doc
%%     Count number of bits used to represent a non negative integer
%% @end
%%--------------------------------------------------------------------
-spec size(X::unsigned()) -> unsigned().

size(X) -> size_(X).

size_(0) -> %% defined to 0, but we really need one bit!
    0;
size_(X) when is_integer(X), X > 0 ->
    size32_(X,0).

size32_(X, I) ->
    if X > 16#FFFFFFFF -> size32_(X bsr 32, I+32);
       true -> size8_(X, I)
    end.

size8_(X, I) ->
    if X > 16#FF -> size8_(X bsr 8, I+8);
       X >= 2#10000000 -> I+8;
       X >= 2#1000000 -> I+7;
       X >= 2#100000 -> I+6;
       X >= 2#10000 -> I+5;
       X >= 2#1000 -> I+4;
       X >= 2#100 -> I+3;
       X >= 2#10 -> I+2;
       X >= 2#1 -> I+1;
       true -> I
    end.
%%--------------------------------------------------------------------
%% @doc
%%     Reverse bits in an unsigned number
%% @end
%%--------------------------------------------------------------------
-spec reverse(X::unsigned()) -> unsigned().
		      
reverse(X) -> reverse_(X).
reverse_(X) when is_integer(X), X >= 0 ->
    reverse__(X, 0).

reverse__(0, Y) ->
    Y;
reverse__(X, Y) ->
    reverse__(X bsr 1, (Y bsl 1) bor (X band 1)).

reverse(X, N) -> reverse_(X, N).
reverse_(X, N) when is_integer(X), X >= 0, is_integer(N), N >= 0 ->
    reverse__(X, N, 0).

reverse__(_, 0, Y) ->
    Y;
reverse__(X, I, Y) ->
    reverse__(X bsr 1, I-1, (Y bsl 1) bor (X band 1)).

%%--------------------------------------------------------------------
%% @doc
%%     Count number of bits set in an unsigned number
%% @end
%%--------------------------------------------------------------------
-spec count(X::unsigned()) -> unsigned().

count(X) -> count_(X).
count_(X) when is_integer(X), X >= 0 ->
    count__(X, 0).

count__(0, I) -> 
    I;
count__(X, I) ->
    count__(X bsr 1, I+(X band 1)).
    
    
%%--------------------------------------------------------------------
%% @doc
%%     Check if a number is a power of 2
%% @end
%%--------------------------------------------------------------------

is_power_of_two(X) when is_integer(X), X > 0 ->
    X band (X-1) =:= 0.

    
