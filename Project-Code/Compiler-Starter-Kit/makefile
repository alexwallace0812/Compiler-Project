all:	compiler.o error_handler.o lille_exception.o scanner.o id_table.o symbol.o token.o parser.o lille_type.o lille_kind.o
	g++ -o compiler compiler.o error_handler.o lille_exception.o scanner.o symbol.o token.o parser.o lille_type.o lille_kind.o
	echo Compilation complete.

compiler.o:	id_table.o error_handler.o lille_exception.o scanner.o symbol.o compiler.cpp
	g++ -std=c++20 -c compiler.cpp

error_handler.o: lille_exception.o token.o error_handler.h error_handler.cpp
	g++ -std=c++20 -c error_handler.cpp

id_table.o:: id_table.h id_table.cpp
	g++ -std=c++20 -c id_table.cpp

lille_exception.o:	lille_exception.h lille_exception.cpp
	g++ -std=c++20 -c lille_exception.cpp

lille_type.o: lille_type.h lille_type.cpp
	g++ -std=c++20 -c lille_type.cpp

lille_kind.o: lille_kind.h lille_kind.cpp
	g++ -std=c++20 -c lille_kind.cpp

scanner.o: error_handler.o lille_exception.o token.o symbol.o id_table.o scanner.h scanner.cpp
	g++ -std=c++20 -c scanner.cpp

parser.o: error_handler.o lille_exception.o token.o symbol.o id_table.o scanner.o parser.h parser.cpp
	g++ -std=c++20 -c parser.cpp

symbol.o: symbol.h symbol.cpp
	g++ -std=c++20 -c symbol.cpp

token.o: lille_exception.o symbol.o token.h token.cpp
	g++ -std=c++20 -c token.cpp

clean:
	rm *.o 
	echo Clean complete
