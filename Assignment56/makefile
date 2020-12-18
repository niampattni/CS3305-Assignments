build: assignment56.c
	gcc -pthread assignment56.c -o a6
test:
	@echo "Running a6 on assignment_6_input_file.txt"
	./a6 assignment_6_input_file.txt
	diff assignment_6_output_file.txt expected_output.txt
	@echo "Passed!"

clean:
	@echo "Cleaning up..."
	rm -f a6 assignment_6_output_file.txt
