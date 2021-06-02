# Курс по системно програмиране за Linux

В това хранилище се споделят материали за лекциите и лабораторните упражнения по системно програмиране за Linux.

Pull requests са винаги добре дошли.

## Генериране на PDF
Презентациите са написани на Latex и Beamer. Командите за генериране на PDF са:

	# For Debian:
	$ sudo apt install texlive-full
	# For Fedora:
	$ sudo dnf install texlive-scheme-full

	# Generate PDF
	$ make
	$ okular *.pdf
