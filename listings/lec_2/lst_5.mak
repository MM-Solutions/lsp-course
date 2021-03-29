${OUT}: ${OBJ}
	@echo "Compile final output"
	${CC} ${LFLAGS} $< -o $@

${OBJ}: ${SRC_LIST}
	@echo "Produce object files"
	${CC} -c $<

clean:
	@echo "Performing cleanup"
	-rm -vf *.o ${OUT}
