run:	main
	@./build/vm

main:
	@mkdir -p build
	
	@g++ main.cpp -lncurses -lpthread -o ./build/vm
