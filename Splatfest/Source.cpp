#include <Python.h>
#include <iostream>
#include <stdio.h>
#include <SFML/Graphics.hpp>
#include <fstream>
#include <vector>

void splatnet2statinkPython();
void drawWindow();
void writeToInputFile();
void copyStringToClipboard(std::string s);
std::string last_token(std::string str);

sf::RenderWindow* window;
std::string textInput = "";
sf::RectangleShape copyBox;
bool clickedCopy = false;
bool copied = false;
PyThreadState* thread1;

int main() {

	window = new sf::RenderWindow(sf::VideoMode(1280, 720), "Splatfest!");
	window->setFramerateLimit(60);
	window->setActive(false);
	copyBox = sf::RectangleShape(sf::Vector2f(200, 100));

	Py_Initialize();

	std::string stdOutErr =
		"import sys\n\
sys.stdout = open('data.txt', 'w')\n\
sys.stdin = open('input.txt','r')\n\
";

	PyObject* pModule = PyImport_AddModule("__main__"); //create main module
	PyRun_SimpleString(stdOutErr.c_str()); //invoke code to redirect


	PyEval_InitThreads();
	Py_DECREF(PyImport_ImportModule("threading"));

	sf::Thread pythonThread1(&splatnet2statinkPython);
	sf::Thread printingThread(&drawWindow);

	pythonThread1.launch();
	printingThread.launch();

	PyThreadState* state = PyEval_SaveThread();

	while (window->isOpen()) {
		sf::Event event;
		while (window->pollEvent(event)) {
			if (event.type == sf::Event::Closed) {
				window->close();
			} else if (event.type == sf::Event::TextEntered) {
				if (event.text.unicode >= 32 && event.text.unicode < 128) {
					textInput += event.text.unicode;
				}

			} else if (event.type == sf::Event::KeyPressed) {
				if (event.key.code == sf::Keyboard::Enter) {
					writeToInputFile();
					textInput = "";
				} else if (event.key.code == sf::Keyboard::BackSpace) {
					textInput = textInput.substr(0, textInput.length() - 1);
				} else if (event.key.control && event.key.code == sf::Keyboard::V) {
					textInput += sf::Clipboard::getString();
				}
			} else if (event.type == sf::Event::MouseButtonPressed) {
				if (event.mouseButton.button == sf::Mouse::Left) {
					int mX = event.mouseButton.x;
					int mY = event.mouseButton.y;

					if (mX >= copyBox.getPosition().x && mX < copyBox.getPosition().x + copyBox.getSize().x &&
						mY >= copyBox.getPosition().y && mY < copyBox.getPosition().y + copyBox.getSize().y) {
						clickedCopy = true;
					}
				}
			}
		}
	}

	thread1->async_exc = PyExc_OSError;
	// Raises an exception in the thread to force it off


	pythonThread1.wait();
	printingThread.wait();

	PyEval_RestoreThread(state);

	Py_Finalize();

	// Clear input.txt
	std::ofstream ofs;
	ofs.open("input.txt", std::ofstream::out | std::ofstream::trunc);
	ofs.close();

	return 0;
}


void splatnet2statinkPython() {

	PyGILState_STATE state = PyGILState_Ensure();

	FILE* pFile;
	pFile = fopen("splatnet2statink-master/splatnet2statink.py", "r");

	wchar_t const* dummy_args[] = { L"splatnet2statink-master/splatnet2statink.py", L"-M", L"60" };
	wchar_t const** argv = dummy_args;
	int argc = 3;

	PyObject* pModule = PyImport_AddModule("__main__");

	PyRun_SimpleString("import sys");
	PyRun_SimpleString("sys.path.append(\"splatnet2statink-master\")");
	PyRun_SimpleString(
		"if not hasattr(sys, 'argv'):\
		sys.argv = ['']");

	thread1 = PyThreadState_Get();

	PySys_SetArgv(argc, const_cast<wchar_t**>(argv));

	PyRun_AnyFile(pFile, "splatnet2statink.py");

	PyGILState_Release(state);

}

void drawWindow() {
	window->setActive(true);

	sf::Font font;
	font.loadFromFile("Arial.ttf");
	std::string prevBattle = "";

	while (window->isOpen()) {

		window->clear();

		sf::RectangleShape bg(sf::Vector2f(1280, 720));
		bg.setFillColor(sf::Color::White);
		window->draw(bg);

		std::ifstream file;
		file.open("data.txt");
		std::string line;
		std::vector<std::string> totalLines;
		while (std::getline(file, line)) {
			totalLines.push_back(line);
		}
		file.close();

		for (int i = 0; i < totalLines.size(); i++) {
			if (totalLines[i].substr(0, 15) == "Battle uploaded") {
				prevBattle = totalLines[i - 1].substr(29, totalLines[i - 1].size());
			}
		}

		if (totalLines.size() > 0) {
			if (totalLines[totalLines.size() - 1] == "stat.ink API key: ") {

				// Render for API key

				sf::Text text("Please input your stat.ink API key", font);
				text.setCharacterSize(36);
				text.setFillColor(sf::Color::Black);
				text.setPosition(window->getSize().x / 2 - text.getGlobalBounds().width / 2, window->getSize().y / 2 - text.getGlobalBounds().height / 2);
				window->draw(text);
			} else if (totalLines[totalLines.size() - 1] == "Invalid stat.ink API key. Please re-enter it below.") {

				// Render for invalid API key

				sf::Text text("Invalid stat.ink API key. Please re-enter it below.", font);
				text.setCharacterSize(36);
				text.setFillColor(sf::Color::Black);
				text.setPosition(window->getSize().x / 2 - text.getGlobalBounds().width / 2, window->getSize().y / 2 - text.getGlobalBounds().height / 2);
				window->draw(text);
			} else if (totalLines[totalLines.size() - 1] == "Default locale is en-US. Press Enter to accept, or enter your own (see readme for list).") {

				// Render for choosing language

				sf::Text text("Please enter your language (en-US)", font);
				text.setCharacterSize(36);
				text.setFillColor(sf::Color::Black);
				text.setPosition(window->getSize().x / 2 - text.getGlobalBounds().width / 2, window->getSize().y / 2 - text.getGlobalBounds().height / 2);
				window->draw(text);
			} else if (totalLines[totalLines.size() - 1] == "Invalid language code. Please try entering it again.") {

				// Render for invalid language

				sf::Text text("Invalid language code. Please try entering it again.", font);
				text.setCharacterSize(36);
				text.setFillColor(sf::Color::Black);
				text.setPosition(window->getSize().x / 2 - text.getGlobalBounds().width / 2, window->getSize().y / 2 - text.getGlobalBounds().height / 2);
				window->draw(text);
			} else if (totalLines[totalLines.size() - 1] == "Log in, right click the \"Select this person\" button, copy the link address, and paste it below:") {

				// Render for logging in (has button to copy)

				sf::Text text("Click the button below to copy the link, \npaste it in your browser, log in, right click the\n \"Select this person\" button, \ncopy the link address, and paste it below", font);
				text.setCharacterSize(36);
				text.setFillColor(sf::Color::Black);

				text.setPosition(window->getSize().x / 2 - text.getGlobalBounds().width / 2, window->getSize().y / 2 - text.getGlobalBounds().height / 2);
				window->draw(text);

				copyBox.setPosition(text.getGlobalBounds().left + text.getGlobalBounds().width / 2 - copyBox.getSize().x / 2, text.getGlobalBounds().top + text.getGlobalBounds().height + 10);
				copyBox.setFillColor(sf::Color::Green);
				window->draw(copyBox);

				sf::Text copyBoxText("Copy", font);

				if (clickedCopy) {
					copyBoxText.setString("Copied!");
					if (!copied) {
						copyStringToClipboard(totalLines[totalLines.size() - 2]);
						copied = true;
					}
				}


				copyBoxText.setCharacterSize(36);
				copyBoxText.setFillColor(sf::Color::White);
				copyBoxText.setPosition(copyBox.getPosition().x + copyBox.getSize().x / 2 - copyBoxText.getGlobalBounds().width / 2, copyBox.getPosition().y + copyBox.getSize().y / 2 - copyBoxText.getGlobalBounds().height / 2);

				window->draw(copyBoxText);
			} else if (totalLines[totalLines.size() - 1] == "URL: ") {

				// Render for an invalid entered URL

				sf::Text text("Invalid URL. Try again.", font);
				text.setCharacterSize(36);
				text.setFillColor(sf::Color::Black);
				text.setPosition(window->getSize().x / 2 - text.getGlobalBounds().width / 2, window->getSize().y / 2 - text.getGlobalBounds().height / 2);
				window->draw(text);

			} else if (totalLines[totalLines.size() - 1].substr(0, 21) == "Press Ctrl+C to exit.") {

				// Render for countdown

				sf::Text text("Time until refresh: " + last_token(totalLines[totalLines.size() - 1]) + "s", font);
				text.setCharacterSize(36);
				text.setFillColor(sf::Color::Black);
				text.setPosition(window->getSize().x / 2 - text.getGlobalBounds().width / 2, window->getSize().y / 2 - text.getGlobalBounds().height / 2);

				if (prevBattle != "") {
					text.setString(text.getString() + "\nPrevious Battle: " + prevBattle);
				}


				window->draw(text);

				// We can now remove all the old data from the file
				// since, the data should now be changing dynamically

				if (last_token(totalLines[totalLines.size() - 1]) == "30") {
					// Wipe the data file when there are 30 seconds until next search
					std::ofstream ofs;
					ofs.open("data.txt", std::ofstream::out | std::ofstream::trunc);
					ofs << totalLines[totalLines.size() - 1].c_str();
					ofs.close();
				}

			}

		}

		sf::RectangleShape inputFieldBG(sf::Vector2f(1200, 30));
		inputFieldBG.setFillColor(sf::Color::Black);
		inputFieldBG.setPosition(window->getSize().x / 2 - inputFieldBG.getSize().x / 2, window->getSize().y - 5 - inputFieldBG.getSize().y);

		sf::Text inputFieldText(textInput, font);
		inputFieldText.setCharacterSize(24);
		inputFieldText.setFillColor(sf::Color::White);
		inputFieldText.setPosition(inputFieldBG.getPosition() + sf::Vector2f(5, 0));

		window->draw(inputFieldBG);
		window->draw(inputFieldText);

		window->display();
	}

	window->setActive(false);

}

void writeToInputFile() {
	std::ofstream file;
	file.open("input.txt", std::ios_base::app);
	file << (textInput + "\n").c_str();
	file.close();
}

void copyStringToClipboard(std::string s) {
	sf::Clipboard::setString(s);
}

std::string last_token(std::string str) {
	while (!str.empty() && std::isspace(str.back())) str.pop_back(); // remove trailing white space

	const auto pos = str.find_last_of(" \t\n"); // locate the last white space

	// if not found, return the entire string else return the tail after the space
	return pos == std::string::npos ? str : str.substr(pos + 1);
}