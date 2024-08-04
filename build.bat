bin\compiler bin\main.a -iinc main.lua
bin\compiler bin\math.a mathImpl.lua
bin\linker bin\main.o -lbin\stdlib bin\main.a bin\math.a
