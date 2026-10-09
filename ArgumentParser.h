// ArgumentParser.h -- minimal command-line reader, used by GeneratePileUp.cpp
//
// Keeps the arguments of the program (without its name) as a list of tokens and looks
// the options up by their exact name. The file has no include guard, and <string> and
// <vector> must be included before it.

#include <algorithm>

class ArgumentParser {
private:
  std::vector<std::string> tokens;

public:
  // Stores argv[1] ... argv[argc-1].
  ArgumentParser(int argc, char *argv[]) {
    for (int i = 1; i < argc; ++i)
      this->tokens.push_back(std::string(argv[i]));
  }

  // True if `option` is one of the tokens.
  bool findOption(const std::string &option) const {
    return std::find(this->tokens.begin(), this->tokens.end(), option) != this->tokens.end();
  }

  // String value: the token that follows `option`. Empty string if the option is
  // absent or is the last token.
  void getArgument(const std::string &option, std::string &arg) const {
    static const std::string empty_string("");
    arg = empty_string;
    std::vector<std::string>::const_iterator itr;
    itr = std::find(this->tokens.begin(), this->tokens.end(), option);
    if (itr != this->tokens.end() && ++itr != this->tokens.end()) {
      arg = *itr;
    }
  }

  // Integer value: the token that follows `option`, converted with stoi. `arg` is left
  // unchanged if the option is absent or is the last token.
  void getArgument(const std::string &option, int &arg) const {
    std::vector<std::string>::const_iterator itr;
    itr = std::find(this->tokens.begin(), this->tokens.end(), option);
    if (itr != this->tokens.end() && ++itr != this->tokens.end()) {
      arg = stoi(*itr);
    }
  }

  // List value: all the tokens that follow `option`, up to the next token that starts
  // with "-". `args` is left unchanged if the option is absent. (The first clause of
  // the `for` is a comparison without effect: the loop starts at the option found.)
  void getArgument(const std::string &option, std::vector<std::string> &args) const {
    std::vector<std::string>::const_iterator itr;
    itr = std::find(this->tokens.begin(), this->tokens.end(), option);
    for (itr != this->tokens.end(); itr != this->tokens.end(); ++itr) {
      std::string s = *itr;
      if (s == option)
        continue;
      if (s.find("-") == 0)
        break;
      args.push_back(s);
    }
  }
};