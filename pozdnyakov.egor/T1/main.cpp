#include <algorithm>
#include <iomanip>
#include <iostream>
#include <limits>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace pozdnyakov
{

  struct Note
  {
    std::string name;
    std::vector< std::string > lines;
    std::vector< std::weak_ptr< Note > > links;

    explicit Note(const std::string &n):
      name(n)
    {}
  };

  using Database = std::unordered_map< std::string, std::shared_ptr< Note > >;

  std::string readWord(std::istream &in)
  {
    std::string s;
    if (!(in >> s)) {
      in.clear();
      throw std::logic_error("read word error");
    }
    return s;
  }

  std::string readQuoted(std::istream &in)
  {
    std::string s;
    if (!(in >> std::quoted(s))) {
      in.clear();
      throw std::logic_error("read quoted error");
    }
    return s;
  }

  void cmdNote(std::istream &in, std::ostream &out, Database &db)
  {
    auto name = readWord(in);
    if (db.count(name)) {
      throw std::logic_error("note already exists");
    }
    db[name] = std::make_shared< Note >(name);
  }

  void cmdLine(std::istream &in, std::ostream &out, Database &db)
  {
    auto name = readWord(in);
    auto text = readQuoted(in);

    auto it = db.find(name);
    if (it == db.end()) {
      throw std::logic_error("note not found");
    }
    it->second->lines.push_back(text);
  }

  void cmdShow(std::istream &in, std::ostream &out, Database &db)
  {
    auto name = readWord(in);

    auto it = db.find(name);
    if (it == db.end()) {
      throw std::logic_error("note not found");
    }
    for (const auto &line : it->second->lines) {
      out << line << "\n";
    }
  }

  void cmdDrop(std::istream &in, std::ostream &out, Database &db)
  {
    auto name = readWord(in);

    auto it = db.find(name);
    if (it == db.end()) {
      throw std::logic_error("note not found");
    }
    db.erase(it);
  }

  void cmdLink(std::istream &in, std::ostream &out, Database &db)
  {
    auto from = readWord(in);
    auto to = readWord(in);

    auto it_from = db.find(from);
    auto it_to = db.find(to);
    if (it_from == db.end() || it_to == db.end()) {
      throw std::logic_error("note not found");
    }

    auto pf = it_from->second;
    auto pt = it_to->second;

    for (const auto &w : pf->links) {
      if (w.lock() == pt) {
        throw std::logic_error("already linked");
      }
    }
    pf->links.push_back(pt);
  }

  void cmdMind(std::istream &in, std::ostream &out, Database &db)
  {
    auto from = readWord(in);

    auto it_from = db.find(from);
    if (it_from == db.end()) {
      throw std::logic_error("note not found");
    }

    for (const auto &w : it_from->second->links) {
      if (auto p = w.lock()) {
        out << p->name << "\n";
      }
    }
  }

  void cmdHalt(std::istream &in, std::ostream &out, Database &db)
  {
    auto from = readWord(in);
    auto to = readWord(in);

    auto it_from = db.find(from);
    auto it_to = db.find(to);
    if (it_from == db.end() || it_to == db.end()) {
      throw std::logic_error("note not found");
    }

    auto pf = it_from->second;
    auto pt = it_to->second;

    auto it = std::remove_if(pf->links.begin(), pf->links.end(), [&](const std::weak_ptr< Note > &w) {
      return w.lock() == pt;
    });

    if (it == pf->links.end()) {
      throw std::logic_error("link not found");
    }
    pf->links.erase(it, pf->links.end());
  }

  void cmdExpired(std::istream &in, std::ostream &out, Database &db)
  {
    auto from = readWord(in);

    auto it = db.find(from);
    if (it == db.end()) {
      throw std::logic_error("note not found");
    }

    int count = 0;
    for (const auto &w : it->second->links) {
      if (w.expired()) {
        count++;
      }
    }
    out << count << "\n";
  }

  void cmdRefresh(std::istream &in, std::ostream &out, Database &db)
  {
    auto from = readWord(in);

    auto it = db.find(from);
    if (it == db.end()) {
      throw std::logic_error("note not found");
    }

    auto &lnks = it->second->links;
    lnks.erase(std::remove_if(lnks.begin(), lnks.end(),
                              [](const std::weak_ptr< Note > &w) {
                                return w.expired();
                              }),
               lnks.end());
  }

}

int main()
{
  pozdnyakov::Database db;

  using cmd_t = void (*)(std::istream &, std::ostream &, pozdnyakov::Database &);
  std::unordered_map< std::string, cmd_t > cmds = {
      {"note", pozdnyakov::cmdNote}, {"line", pozdnyakov::cmdLine},       {"show", pozdnyakov::cmdShow},
      {"drop", pozdnyakov::cmdDrop}, {"link", pozdnyakov::cmdLink},       {"mind", pozdnyakov::cmdMind},
      {"halt", pozdnyakov::cmdHalt}, {"expired", pozdnyakov::cmdExpired}, {"refresh", pozdnyakov::cmdRefresh}};

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
