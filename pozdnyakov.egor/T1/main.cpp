#include <iostream>

int main()
{
  pozdnyakov::Database db;

  using cmd_t = void (*)(std::istream &, std::ostream &, pozdnyakov::Database &);
  std::unordered_map< std::string, cmd_t > cmds = {}

  std::string cmd;
  while (std::cin >> cmd) {
    try {
      auto it = cmds.find(cmd);
      if (it == cmds.end()) {
        throw std::out_of_range("unknown command");
      }
      it->second(std::cin, std::cout, db);
    } catch (const std::out_of_range &) {
      std::cout << "<INVALID COMMAND>\n";
      std::cin.clear();
      std::cin.ignore(std::numeric_limits< std::streamsize >::max(), '\n');
    } catch (const std::logic_error &) {
      std::cout << "<INVALID COMMAND>\n";
      if (std::cin.fail()) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits< std::streamsize >::max(), '\n');
      }
    }
  }

  return 0;
}
